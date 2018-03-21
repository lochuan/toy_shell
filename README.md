# toy_shell

```
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 +   This is a homework shell, I haven't cover the whole  +
 +   function set that most common system shell does yet. +
 +                                                        +
 +   The pipeline and redirection works well, It supports +
 +   multi-pipeline, both input and output redirection.   +
 +                                                        +
 +   i.e.: ls | sort -r | head      //multi-pipeline      +
 +         ls -l > ls.out           //output redirection  +
 +         sort -r < ls.out         //input redirection   +
 +                                                        +
 +   You can also combine them together.                  +
 +                                                        +
 +   i.e.: ls | sort -r | head > example.out              +
 +         wc | sort -r < example.in                      +
 +                                                        +
 +   I missed out lots of details with error catches, if  +
 +   any bugs or crashes occur, please try re-opening it. +
 +                                                        +
 +   Prress CTRL+C to exit!!                              +
 +                                                        +
 +                     ZHANG ZHENGWEN                     +
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 ```
