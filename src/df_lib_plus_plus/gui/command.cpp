// Own header
#include "command.h"

// Project headers
#include "mutex.h"
#include "string_utils.h"
// #include "widgets/gui_manager.h"
// #include "widgets/command_view.h"

// Standard headers
#include <stdlib.h>
#include <string.h>


CommandSender g_commandSender;
char COMMAND_RETURN_NOTHING[] = "nothing";


char *CreateIntToSend(int val)
{
    // Max num chars needed to represent an int is:
    // 10 digits + 1 sign + 1 null terminator = 12
    char *rv = new char[12];
    itoa(val, rv, 10);
    return rv;
}


// ****************************************************************************
// Class CommandReceiver
// ****************************************************************************

CommandReceiver::CommandReceiver(char const *name)
{
	m_receiverName = StringDuplicate(name);
}


CommandReceiver::~CommandReceiver()
{
	delete[] m_receiverName;
}


bool CommandReceiver::GetArgumentBool(char const *arguments, char const *argName, bool defaultVal)
{
	bool val = defaultVal;
	if (arguments)
	{
		char const *match = strstr(arguments, argName);
		if (match)
		{
			match += strlen(argName);
			if (*match == '=')
			{
				match++;
				if (stricmp(match, "true") == 0)
				{
					val = true;
				}
				else
				{
					val = false;
				}
			}
		}
	}

	return val;
}


int CommandReceiver::GetArgumentInt(char const *arguments, char const *argName, int defaultVal)
{
	int val = defaultVal;
	if (arguments)
	{
		char const *match = strstr(arguments, argName);
		if (match)
		{
			match += strlen(argName);
			if (*match == '=')
			{
				match++;
				val = atoi(match);
			}
		}
	}

	return val;
}



// ****************************************************************************
// Class CommandSender
// ****************************************************************************

CommandSender::CommandSender()
{
    m_deferredCommandsMutex = new Mutex;
}


void CommandSender::RegisterReceiver(CommandReceiver *receiver)
{
	DebugAssert(receiver->m_receiverName);
	DebugAssert(strlen(receiver->m_receiverName) > 2);

	// Make sure the specified CommandReceiver isn't already registered
	for (int i = 0; i < m_receivers.Size(); ++i)
	{
		CommandReceiver *r = m_receivers[i];
		if (stricmp(receiver->m_receiverName, r->m_receiverName) == 0)
		{
			DebugAssert(0);
		}
	}

	m_receivers.PutData(receiver);
}


char *CommandSender::SendCommand(char const *from, char const *target, char const *command, char const *arguments)
{
    CommandReceiver *receiver = NULL;

	if (!receiver)
	{
		for (int i = 0; i < m_receivers.Size(); ++i)
		{
			CommandReceiver *thisReceiver = m_receivers[i];
			if (stricmp(thisReceiver->m_receiverName, target) == 0)
			{
				receiver = thisReceiver;
				break;
			}
		}
	}

	if (receiver)
	{
// TODO - re-add CommandHistory
//         CommandHistory *cv = (CommandHistory*)g_guiManager->GetWidgetByName(COMMAND_VIEW_WINDOW_NAME);
//         ReleaseAssert(cv, "CommandSender::SendCommand: command view window not found");
//         cv->AddCommand(from, target, command, arguments);

		return receiver->ExecuteCommand(from, command, arguments);
	}

	return NULL;
}


void CommandSender::SendCommandDeferred(char const *from, char const *target, 
                                         char const *command, char const *arguments)
{
    MutexLocker lock(m_deferredCommandsMutex);
    DeferredCommand *dc = new DeferredCommand(from, target, command, arguments);
    m_deferredCommands.PutDataAtEnd(dc);
}


void CommandSender::ProcessDeferredCommands()
{
    MutexLocker lock(m_deferredCommandsMutex);
    while (m_deferredCommands.Size())
    {
        DeferredCommand *dc = m_deferredCommands[0];
        m_deferredCommands.RemoveData(0);
        SendCommandNoRV(dc->m_from, dc->m_target, dc->m_command, dc->m_arguments);
        delete dc;
    }
}
