Index
1. Creating a new language for RV House
2. Updating language file to new version

-------

1. Creating a new language for RV House
- Copy english.def to <your_language>.def
- Edit the .def file to have your information.
  This information will be seen in RV House Info box
  in bottom (Translations)
- Copy english.lang to <your_language>.def

The language file's structure is following:
[something] - marks a section
id = text - says 'id' equals some text

Edit only the texts. Not the section names or ids. For example
if original .lang file has:

[login_win]
title           = Login
user_id         = &User ID:

You can edit it to Finnish by changing it to:

[login_win]
title           = Kirjaudu
user_id         = &Käyttäjätunnus:


The lang file strings have some symbols that can be used to denote
special functions:
& - marks a shortcut in a dialog window. If this character is
    before a letter, that letter is used as a shortcut. 
    Alt+letter then activates the corresponding controller in
    the dialog. If you want to assign shortcuts, make sure 
    each dialog do not have overlapping shortcuts.
\1 - can be also another number. With some strings, these will be
     replaced with some input from RV House that can vary
     (for example user name etc.). It should be clear from the
     context of the string what kind of data there can be
\n - Marks a new text line. Used in some long texts that are
     displayed in dialogs.

If a line starts with '#' character, it is a comment.

Also the strings can be enclosed in quotes, which can be used for
example to mark significant white space.

-------

2. Updating language file to new version
RV House has functionality that is helpful for bringing
languages up to date when new versions are released.
With the functionality you can easily see if there has
been new strings to translate.

To use it:
- Open command line
- Go to where you installed new version of RV House
- run RV House with parameter --langdiff <your language>
- The missing language texts are shown in the format of:
  section/id, for example room_win/share_tracks
  
An example:
C:\Program Files\RV House>rv_house --langdiff Spanish

Remember to change also the version in language.def
to the new version!
