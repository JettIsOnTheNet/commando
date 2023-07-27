# Commando
Let Commando do the heavy lifting.

Commando (cdo) is your AI terminal command companion. The goal of Commando is to be able to talk to your terminal in natural language.

![commando](images/commando.jpg)

Example:
```
cdo replace all instances of ‘foo’ with ‘bar’ in file.txt using sed
cdo pip remove package # properly calls the uninstall command
cdo grep all files in /var/log for ‘baz’ and cat results to results.txt
```

BIG WARNING:
cdo is a work in progress, and incomplete. This is something I whipped up in 2 hours, so it is not yet finished. If you would like to dig in, please do. It requires libcurl because that was quick to implement however I am aware there are better ways to do it. I am not the best at C, so please feel free to make PR.

TODOLIST from cdo.c
* TODOLIST:
* TODO: better err handling
* TODO: cmd validation, we validate nothing, nothing is valid
* TODO: this is rough and dirty. there is zero async req handling
* TODO: deal /w security and execution
* TODO: potential memory leaks, need to handle them
* TODO: better input handling than the basic regex mess
* TODO: move all quotes and language strings to a file
* TODO: there is no system prompt
* TODO: need to have system prompt be a part of the config file
* TODO: system prompt should tell GPT to wrap cmd
*          -cmd-/:cmd:/@cmd@/*cmd* dunno, char needs to be non volatile to shellcmd
*          this way, each can be parsed, put into arr, and multiple
*          lines can be sent in 1 request, then iter on each cmd
* TODO: user needs to be able to run in free access mode/intervention
*          intervention: require each returned list of cmds to be y/n
*          free access: "It's turbo time!" Guardrails off, good luck.
* TODO: need to specify to LLM/GPT which OS it is running on. Win/Mac/Linux(distro)/BSD
* TODO: add ability to use local/self hosted LLM trained on terminal cmds

Build:
```
g++ -o cdo cdo.cpp -lcurl
```