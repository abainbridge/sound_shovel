#pragma once


// Project headers
#include "string_utils.h"
#include "containers/llist.h"


class Mutex;

extern char COMMAND_RETURN_NOTHING[];

// Helper macros used when writing ExecuteCommand functions
#define COMMAND_IS(a) stricmp(command, a) == 0
#define COMMAND_STARTS(a) StringStartsWith(command, a)


char *CreateIntToSend(int val);
static void FreeCommandResult(char *res)
{
    if (res != COMMAND_RETURN_NOTHING)
    {
        delete[] res;
    }
}


// ****************************************************************************
// Class CommandReceiver
// ****************************************************************************

// Any class that wishes to receive 'commands' should inherit from
// this class. Commands are sent by the KeyboardShortcutManager and MenuBar
class CommandReceiver
{
public:
	char *m_receiverName;		// Guaranteed to be lower case

	CommandReceiver(char const *name);
	~CommandReceiver();

	virtual bool HasFocus() { return false; }
	virtual char *ExecuteCommand(char const *object, char const *command, char const *arguments) = 0;

	static bool GetArgumentBool(char const *arguments, char const *argName, bool defaultVal);
	static int  GetArgumentInt (char const *arguments, char const *argName, int defaultVal);
};


class DeferredCommand
{
public:
    char *m_from;
    char *m_target;
    char *m_command;
    char *m_arguments;

    DeferredCommand(char const *from, char const *target, char const *command, char const *arguments)
    {
        m_from = StringDuplicate(from);
        m_target = StringDuplicate(target);
        m_command = StringDuplicate(command);
        m_arguments = StringDuplicate(arguments);
    }

    ~DeferredCommand()
    {
        delete [] m_from;
        delete [] m_target;
        delete [] m_command;
        delete [] m_arguments;
    }
};


// ****************************************************************************
// Class CommandSender
// ****************************************************************************

class CommandSender
{
private:
	LList <CommandReceiver *> m_receivers;
    LList <DeferredCommand *> m_deferredCommands;
    Mutex *m_deferredCommandsMutex; // Prevents two threads accessing m_deferredCommands at once

public:
    CommandSender();

	void RegisterReceiver(CommandReceiver *receiver);

	// isGlobal states whether or not command should be executed when the CommandReceiver hasn't got focus
	// The return value is a string containing the result of whatever command was called. NULL is returned
	// if no command was executed, but may also be returned by a command that just doesn't want to give
	// a return value. THE CALLER is responsible for DELETING the returned string.
	char *SendCommand(char const *from, char const *target, char const *command, char const *arguments);

    // Wraps SendCommand and deletes any return value
    void SendCommandNoRV(char const *from, char const *target, char const *command, char const *arguments)
    {
        char *rv = SendCommand(from, target, command, arguments);
        FreeCommandResult(rv);
    }

    // Use this to send a command to the main thread from another. The command will be queued.
    // Later the main thread will call ProcessDeferredCommands to unqueue them.
    void SendCommandDeferred(char const *from, char const *target, char const *command, char const *arguments);

    void ProcessDeferredCommands();
};


extern CommandSender g_commandSender;
