
I just remembered that Ed asked me if I could share my Grid application template and figured I should do that before I fall asleep and forget again. I have attached a shell script (the .txt extension prevents Outlook from blocking it...) that will generate a basic Grid application. To set it up, just make it into an executable shell script with e.g.

1. mv gen-grid-program.txt gen-grid-program.sh
2. chmod 744 gen-grid-program.sh

And then you can use it to create an application template,

3. ./gen-grid-program.sh NameOfApplication

This will generate a directory called NameOfApplication in the current working directory, which builds a program also called NameOfApplication. Inside you'll find that it has its own bootstrap and autotools scripts, alongside a README that tells you how to compile the program, and a mainfile inside a 'src' directory. You can then turn this into whatever program you like, presumably by editing the contents of the 'src' directory.

If you add new cpp files, don't forget to add them to the Makefile.am, and if you end up releasing the program, you'll probably want to edit the author information in the AC_INIT at the top of the configure.ac file.

Best,
Ryan
