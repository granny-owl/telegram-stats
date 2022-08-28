# telegram_stats
The program to collect stats about how you use the English language in personal chats (telegram chat statistics collector / analyzer)

Instructions:
- export your chat as JSON file (see "export dialogue" in your telegram app) and place it as "chat.json" in main folder,
- put your name and the name of a person you talk to into main.cpp,
- perform usual cmake build, run the code - voila!

Limitations:
- does NOT filter verb forms (-ing/-ed; counted as different),                      // TO DO
- complataly skips the words with repeated character (YASSSSS == YAS),              // TO DO

Methodology:
- skips photos, videos, sticker and other non-text messages (emoji, punctuation),
- skips links (youtube, fb etc.),
- skips words with non-Latin characters;
- filters capital letters (GOOD == good == gOOd etc.),
- filters basic plurals and posession (Jane == Janes == Jane's),

Results for each participant:
- total message count,
- average word length,
- median word length,
- number of unique words,
- most popular words,
- creates .txt files with words sorted by their frequency + word use count.
