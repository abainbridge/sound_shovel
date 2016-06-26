// Own header
#include "file_dialog.h"

// Project headers
#include "andy_string.h"
#include "darray.h"

// Platform headers
#include <windows.h>
#include <shobjidl.h>


#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


static void WindowsFilePathStr(String *path)
{
    for (int i = 0; i < path->size(); i++)
    {
        if ((*path)[i] == '/')
            (*path)[i] = '\\';
    }
}


static void SetInitialFolder(IFileDialog *fd, char const *initialFolder)
{
    String initialFolderWinStyle = initialFolder;
    WindowsFilePathStr(&initialFolderWinStyle);

    wchar_t widePath[MAX_PATH + 1];
    mbstowcs(widePath, initialFolderWinStyle.c_str(), MAX_PATH);
    IShellItem *siFolder = NULL;
    HRESULT hr = SHCreateItemFromParsingName(widePath, NULL, IID_PPV_ARGS(&siFolder));
    if (SUCCEEDED(hr)) 
    {
        fd->SetFolder(siFolder);
        siFolder->Release();
    }
}


DArray <String> FileDialogOpen(char const *initialFolder)
{
    DArray <String> files;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        // CoCreate the File Open Dialog object.
        IFileOpenDialog *fd = NULL;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fd));
        if (SUCCEEDED(hr))
        {
            if (initialFolder)
                SetInitialFolder(fd, initialFolder);

            // Set the options on the dialog.
            DWORD flags;

            // Before setting, always get the options first in order not to override existing options.
            hr = fd->GetOptions(&flags);
            if (SUCCEEDED(hr))
            {
                // In this case, get shell items only for file system items.
                hr = fd->SetOptions(flags | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT);
                if (SUCCEEDED(hr))
                {
                    // Show the dialog
                    hr = fd->Show(NULL);
                    if (SUCCEEDED(hr))
                    {
                        // Obtain the array of results
                        IShellItemArray *results;
                        hr = fd->GetResults(&results);
                        if (SUCCEEDED(hr))
                        {
                            DWORD numItems;
                            hr = results->GetCount(&numItems);
                            if (SUCCEEDED(hr))
                            {
                                // Iterate through all results and add each to the DArray
                                for (int i = 0; i < numItems; i++)
                                {
                                    IShellItem *result;
                                    hr = results->GetItemAt(i, &result);
                                    if (SUCCEEDED(hr))
                                    {
                                        PWSTR filePath = NULL;
                                        result->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                                        if (SUCCEEDED(hr))
                                        {
                                            // Convert Wide-String to String
                                            size_t size = wcstombs(NULL, filePath, 0);
                                            char *charStr = new char[size + 1];
                                            wcstombs(charStr, filePath, size + 1);

                                            files.Push(String(charStr));
                                            delete charStr;
                                        }
                                        result->Release();
                                    }
                                }
                            }
                            results->Release();
                        }
                    }
                }
            }
            fd->Release();
        }
        CoUninitialize();
    }

    return files;
}


String FileDialogSave(char const *initialFolder, char const *initialFilename)
{
    String file;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        // CoCreate the File Open Dialog object.
        IFileSaveDialog *fd = NULL;
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fd));
        if (SUCCEEDED(hr))
        {
            SetInitialFolder(fd, initialFolder);

            // Set the options on the dialog.
            DWORD flags;

            // Before setting, always get the options first in order not to override existing options.
            hr = fd->GetOptions(&flags);
            if (SUCCEEDED(hr))
            {
                // In this case, get shell items only for file system items.
                hr = fd->SetOptions(flags | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM);
                if (SUCCEEDED(hr))
                {
                    // Set the initial save filename
                    String initialFilenameFull;
                    if (initialFolder[0] != '\0')
                    {
                        initialFilenameFull = initialFolder;
                        if (initialFilenameFull[initialFilenameFull.size() - 1] != '/')
                            initialFilenameFull += "/";
                    }
                    initialFilenameFull += initialFilename;
                    WindowsFilePathStr(&initialFilenameFull);
                    wchar_t widePath[MAX_PATH + 1];
                    mbstowcs(widePath, initialFilenameFull.c_str(), MAX_PATH);
                    IShellItem *siFilename = NULL;
                    hr = SHCreateItemFromParsingName(widePath, NULL, IID_PPV_ARGS(&siFilename));
                    if (SUCCEEDED(hr))
                    {
                        fd->SetSaveAsItem(siFilename);
                        siFilename->Release();
                    }

                    // Show the dialog
                    hr = fd->Show(NULL);
                    if (SUCCEEDED(hr))
                    {
                        // Obtain the array of results
                        IShellItem *result;
                        hr = fd->GetResult(&result);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR filePath = NULL;
                            result->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                            if (SUCCEEDED(hr))
                            {
                                // Convert Wide-String to String
                                size_t size = wcstombs(NULL, filePath, 0);
                                char *charStr = new char[size + 1];
                                wcstombs(charStr, filePath, size + 1);

                                file = charStr;
                                delete charStr;
                            }
                            result->Release();
                        }
                    }
                }
            }
            fd->Release();
        }
        CoUninitialize();
    }

    return file;
}
