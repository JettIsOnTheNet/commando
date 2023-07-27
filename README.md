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

Build:
```
g++ -o cdo cdo.cpp -lcurl
```