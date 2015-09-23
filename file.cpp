#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <queue>
#include <string.h>

using namespace std;

struct arg
{
    string arg1, arg2, arg3, arg4;
};

struct instruction
{
    vector<char *> args;
    bool isNextPipe;
    string readFile;
    string writeFile;
    int in, out;
};

int printVector(vector<string> v)
{
    for(int i = 0; i < v.size(); i++)
        cout << "arg: " << v[i] << endl;
    cout << endl;

    return 0;
}

char* convert(string str)
{
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    
    return cstr;
}

queue<instruction> getInput()
{
    string input;
    string temp;
    vector<string> tempArg;
    queue<instruction> forks;
    cout << "~: ";
    getline(cin, input);
    for(int i = 0; i < input.size(); i++)
    {
        if(input[i] != ' ' && input[i] != '|' && input[i] != '<' && input[i] != '>')
        {
            temp += input[i];
            if(i == input.size()-1)
            {
                tempArg.push_back(temp);
                temp = "";
            }
        }    
        else if( input[i] == ' ')
        {
            if(temp.size() > 0)
                tempArg.push_back(temp);
            temp = "";
        }
        else if(input[i] == '|')
        {
            temp = input[i];
            tempArg.push_back(temp);
            temp = "";
        }
        else if(input[i] == '<' || input[i] == '>')
        {
            temp = input[i];
            tempArg.push_back(temp);
            temp = "";
        }
    }
    
    //printVector(tempArg);

    while(!tempArg.empty())
    {
        instruction I;
        I.isNextPipe = false;
        I.readFile = "";
        I.writeFile = "";
        I.in = 0;
        I.out = 0;
        if(tempArg[0] != "|")
            I.args.push_back(convert(tempArg[0]));
        tempArg.erase(tempArg.begin());
        if(tempArg[0] == "-l")
        {
            I.args.push_back(convert(tempArg[0]));
            tempArg.erase(tempArg.begin(), tempArg.begin()+1);
            if(tempArg[0] == ">")
            {
                I.writeFile = tempArg[1];
                tempArg.erase(tempArg.begin(), tempArg.begin()+2);
            }

            else if(tempArg[0] == "<")
            {
                I.readFile = tempArg[1];
                tempArg.erase(tempArg.begin(), tempArg.begin()+2);
            }
        }
        else if(tempArg[0] == ">")
        {
            I.writeFile = tempArg[1];
            tempArg.erase(tempArg.begin(), tempArg.begin()+2);
        }
        else if(tempArg[0] == "<")
        {
            I.readFile = tempArg[1];
            tempArg.erase(tempArg.begin(), tempArg.begin()+2);
        }
        else if(tempArg[0] == "|")
            I.isNextPipe = true;
        /*
        if(I.args.arg1 != "")
            cout << "arg1: " << I.args.arg1 << "   repeat arg1: "\
                << I.args.arg1 << "   arg2: " << I.args.arg2 << "    arg3: "\
                << I.args.arg3 << "    arg4: " << I.args.arg4 << endl;
        cout << "-----------------------------------------------------------------------" << endl;
        cout << endl;
        cout << endl;
        //printVector(tempArg);
        */
        if(!I.args.empty())
        {
            I.args.push_back(NULL);
            forks.push(I);   
        }
    }
    return forks;
}

/*
void printQueue(queue<instruction> steps)
{
    
    while(!steps.empty())
    {
        instruction I = steps.front();
        steps.pop();
        cout << "Args:" << endl;
        cout << "   arg1: " << I.args.arg1 << "    arg1 repeated: " <<\
            I.args.arg1 << "    arg2: " << I.args.arg2 << "     arg3: "\
            << I.args.arg3 << "    arg4: " << I.args.arg4 << endl;
        cout << endl;
        cout << "Pipe:" << endl;
        cout << "   " << I.isNextPipe << endl;
        cout << endl;
        cout << "readFile: " << endl;
        cout << "   " << I.readFile << endl;
        cout << endl;
        cout << "writeFile: " << endl;
        cout << "   " << I.writeFile << endl;
        cout << endl;
        cout << "in: " << endl;
        cout << "   " << I.in << endl;
        cout << endl;
        cout << "out: " << endl;
        cout << "   " << I.out << endl;
        cout << "------------------------------------------------------"\
            << endl;

        cout << endl;
    }   
}
*/
int beginFork(instruction I)
{
    
    int pfd1[2];
    if(pipe(pfd1) == -1){
        fprintf(stderr, "Pipe Failed");
        exit(1);
    }
    if(I.readFile != "")
    {
        int readFile;
        if((readFile = open(I.readFile.c_str(), O_RDONLY)) < 0)
        {
            cout << "File could not be opened!" << endl;
            exit(1);
        }
        I.in = readFile;
        close(readFile);
    }
    
    pid_t childPID;
    switch(childPID = fork()){
        case -1: 
            fprintf(stderr, "Fork Failed");
	    exit(1);
        case 0:
            if(dup2(I.in, 0) == -1 || dup2(I.out, 1) == -1)
            {
                cout << "Errors in first Child Process!" << endl;
                exit(1);
            }
            char **command = &I.args[0];
            execvp(command[0], command);
    }
    
    return 0;
}

int midFork(instruction I)
{
    pid_t second_childPID;
    switch(second_childPID = fork()){
        case -1: 
            fprintf(stderr, "Fork Failed");
	    exit(1);
        case 0:
            if(dup2(I.in, 0) == -1 || dup2(I.out, 1) == -1)
            {
                cout << "Errors in second Child Process!" << endl;
                exit(1);
            }
            char **command = &I.args[0];
            execvp(command[0], command);
    }
    
    return 0;
}

int endFork(instruction I)
{
    if(I.writeFile != "")
    {
        int writeFile;
        if((writeFile = open(I.writeFile.c_str(), O_WRONLY)) < 0)
        {
            cout << "File could not be opened!" << endl;
            exit(1);
        }
        I.out = writeFile;
        close(writeFile);
    }
    
    pid_t third_childPID;
    switch(third_childPID = fork()){
        case -1: 
            fprintf(stderr, "Fork Failed");
	    exit(1);
        case 0:
            if(dup2(I.in, 0) == -1 || dup2(I.out, 1) == -1)
            {
                cout << "Errors in third Child Process!" << endl;
                exit(1);
            }
            char **command = &I.args[0];
            execvp(command[0], command);
    }
    
    return 0;
}

int basicFork(instruction I)
{
    if(I.readFile != "")
    {
        int readFile;
        if((readFile = open(I.readFile.c_str(), O_RDONLY)) < 0)
        {
            cout << "File could not be opened!" << endl;
            exit(1);
        }
        I.in = readFile;
        close(readFile);
    }
    if(I.writeFile != "")
    {
        int writeFile;
        if((writeFile = open(I.writeFile.c_str(), O_WRONLY)) < 0)
        {
            cout << "File could not be opened!" << endl;
            exit(1);
        }
        I.out = writeFile;
        close(writeFile);
    }
    
    pid_t fourth_childPID;
    switch(fourth_childPID = fork()){
        case -1: 
            fprintf(stderr, "Fork Failed");
	    exit(1);
        case 0:
            if(dup2(I.in, 0) == -1 || dup2(I.out, 1) == -1)
            {
                cout << "Errors in fourth Child Process!" << endl;
                exit(1);
            }
            char **command = &I.args[0];
            execvp(command[0], command);
    }
    
    return 0;
}

int main()
{
    queue<instruction> steps = getInput();
    bool pipeSwap, firstPipe;
    bool pipeStatus = false;
    instruction I = steps.front();
    
    // Determines if there needs to be a pipe or not
    if(I.isNextPipe)
    {
        pipeStatus = true;
        firstPipe = true;
    }
    
    // Creates pipe
    int pfd1[2];
    if(pipe(pfd1) == -1){
        fprintf(stderr, "Pipe Failed");
        exit(1);
    }
    int pfd2[2];
    if(pipe(pfd2) == -1){
        fprintf(stderr, "Pipe 2 Failed!");
    }

    // Goes through until there are no instructions left
    while(!steps.empty())
    {
        I = steps.front();
        
        // If no pipe, does a basic fork
        if(!pipeStatus)
        {
            basicFork(I);        
        }

        // If there is a pipe, determines what file descriptors need access to what pipes
        else
        {
            if(firstPipe)
            {
                I.out = pfd1[1];
                firstPipe = false;
                beginFork(I);
            }
            else if(I.isNextPipe)
            {
                if(pipeSwap)
                {
    
                    I.in = pfd1[0];
                    I.out = pfd1[1];
                    pipeSwap = false;
                }
                else
                {
                    I.in = pfd1[1];
                    I.out = pfd1[0];
                    pipeSwap = true;
                }
                midFork(I);
            }
            else if(!I.isNextPipe)
            {
                if(pipeSwap)
                {
                    I.in = pfd1[0];
                    pipeSwap = false;
                }
                else
                {
                    I.in = pfd1[1];
                    pipeSwap = true;
                }
                endFork(I);
            }
        }
        steps.pop();
    }
    
    while( wait( ( int *) 0) != -1)
        ;
 
    close(pfd1[0]);
    close(pfd2[0]);
    close(pfd1[1]);
    close(pfd2[1]);
    
    return 0;
}
