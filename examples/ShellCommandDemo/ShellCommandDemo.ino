/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-27
 * @brief Example of a shell command processor based on
 *        the Nordic UART Service
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuShellCommands.hpp"
#include <NimBLEDevice.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

//------------------------------------------------------
// Shell Commands implementation for a simple calculator
//------------------------------------------------------

class MyShellCommandCallbacks : public NuShellCommandCallbacks
{
public:
    // commands are implemented by overriding the following methods

    virtual void onExecute(NuShellCommand_t &commandLine) override;
    virtual void onParseError(NuShellParsingResult_t parsingResult) override;

private:
    // macro to print a number into the BLE serial port
    static void printNumberResponse(intmax_t number);

    // macro to parse a string into an integer
    static bool strToIntMax(const char text[], intmax_t &number);

} myShellCallbacks; // just one global instance is needed

//------------------------------------------------------

void MyShellCommandCallbacks::printNumberResponse(intmax_t number)
{
    char buffer[64]; // should be enough for a single integer number
    snprintf(buffer, 64, "%lld", number);
    buffer[63] = '\0';
    NuShellCommands.send(buffer);
}

bool MyShellCommandCallbacks::strToIntMax(const char text[], intmax_t &number)
{
    // "errno" is used to detect non-integer data
    // errno==0 means no error
    errno = 0;
    intmax_t r = strtoimax(text, NULL, 10);
    if (errno == 0)
    {
        number = r;
        return true;
    }
    else
        return false;
}

// Commands
#define CMD_UNKNOWN 0
#define CMD_ADD 1
#define CMD_SUB 2
#define CMD_MULT 3
#define CMD_DIV 4

void MyShellCommandCallbacks::onExecute(NuShellCommand_t &commandLine)
{
    Serial.println("--New command line--");
    for (const char *str : commandLine)
    {
        Serial.println(str);
    }
    Serial.println("--End of command line--");

    if (commandLine.size() == 3)
    {
        const char *commandName = commandLine[0];
        const char *op1Str = commandLine[1];
        const char *op2Str = commandLine[2];

        int commandId = CMD_UNKNOWN;
        if ((strcmp(commandName, "ADD") == 0) || (strcmp(commandName, "add") == 0))
            commandId = CMD_ADD;
        else if ((strcmp(commandName, "SUM") == 0) || (strcmp(commandName, "sum") == 0))
            commandId = CMD_ADD;
        else if ((strcmp(commandName, "SUBTRACT") == 0) || (strcmp(commandName, "subtract") == 0))
            commandId = CMD_SUB;
        else if ((strcmp(commandName, "SUB") == 0) || (strcmp(commandName, "sub") == 0))
            commandId = CMD_SUB;
        else if ((strcmp(commandName, "MULT") == 0) || (strcmp(commandName, "mult") == 0))
            commandId = CMD_MULT;
        else if ((strcmp(commandName, "DIVIDE") == 0) || (strcmp(commandName, "divide") == 0))
            commandId = CMD_DIV;
        else if ((strcmp(commandName, "DIV") == 0) || (strcmp(commandName, "div") == 0))
            commandId = CMD_DIV;

        if (commandId != CMD_UNKNOWN)
        {
            intmax_t op1 = 0LL;
            intmax_t op2 = 0LL;
            if (!strToIntMax(op1Str, op1))
            {
                NuShellCommands.send("ERROR: 1st argument is not a valid integer\n");
            }
            else if (!strToIntMax(op2Str, op2))
            {
                NuShellCommands.send("ERROR: 2nd argument is not a valid integer\n");
            }
            else
            {
                switch (commandId)
                {
                case CMD_ADD:
                    printNumberResponse(op1 + op2);
                    break;
                case CMD_SUB:
                    printNumberResponse(op1 - op2);
                    break;
                case CMD_MULT:
                    printNumberResponse(op1 * op2);
                    break;
                case CMD_DIV:
                    if (op2 != 0)
                        printNumberResponse(op1 / op2);
                    else
                        NuShellCommands.send("ERROR: divide by zero\n");
                    break;
                }
            }
        }
        else
        {
            NuShellCommands.send("ERROR: unknown command\n");
        }
    }
    else
    {
        NuShellCommands.send("ERROR: expected a command and two arguments\n");
    }
}

void MyShellCommandCallbacks::onParseError(NuShellParsingResult_t parsingResult)
{
    NuShellCommands.send("SYNTAX ERROR. Code: ");
    printNumberResponse(parsingResult);
}

//------------------------------------------------------
// Arduino entry points
//------------------------------------------------------

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("*******************************");
    Serial.println(" BLE AT command processor demo ");
    Serial.println("*******************************");
    Serial.println("--Initializing--");

    // Initialize BLE and Nordic UART service
    NimBLEDevice::init("Shell commands demo");
    NuShellCommands.setBufferSize(128);
    NuShellCommands.setShellCommandCallbacks(&myShellCallbacks);
    NuShellCommands.start();

    // Initialization complete
    Serial.println("--Ready--");
}

void loop()
{
    // Incoming data is processed in another task created by the BLE stack,
    // so there is nothing to do here (in this demo)
    Serial.println("--Running (heart beat each 30 seconds)--");
    delay(30000);
}