
TO ADD -
LookAhead(1) but called  peekchar():
Instead of reading directly, use a getchar(): 
devuelve el siguiente carácter del input y mueve el puntero del carácter al siguiente.  
Function  gettoken(): to call the scanner.
Properly display error(s) by line found and type of error.
Add the calling of FUNCTIONS. 

SCANNER MUST BE!!!!
"Smart":  Incluir una funcionalidad de salida detallada que rastree las etapas del scanner.

INFO SCAN - Start scanning… 
DEBUG SCAN - ID [ a ] found at (1:1) 
DEBUG SCAN - ASSIGN_OP [ = ] found at (1:3)
DEBUG SCAN - INT [ 2 ] found at (1:5) 
DEBUG SCAN - ADD_OP [ + ] found at (1:7) 
DEBUG SCAN - INT [ 7 ] found at (1:8) 
DEBUG SCAN - PRINT_KEY [ print ] found at (2:1) 
DEBUG SCAN - OPEN_PAR [ ( ] found at (2:7) 
DEBUG SCAN - ID [ a ] found at (2:8) 
DEBUG SCAN - CLOSE_PAR [ ) ] found at (2:9) 
DEBUG SCAN - EOP [ $ ] found at (3:1) 
INFO SCAN - Completed with 0 errors   

*/
//GRAMMAR
/*
<program>   ::= <statement> | <statement> <program>
<statement> ::= <assign> | <command> | <if>
<assign>    ::= "let" <ID> "=" <expression> ";"
<command>   ::= <extract_frame> | <concatenate> | <extract_audio> | <play>
    <extract_frame> ::= "frame" <expression> <expression> "to" <string> ";"
    <concatenate>   ::= "concat" <expression> <expression> "to" <string> ";"
    <extract_audio> ::= "audio" <expression> <expression> <expression> "to" <string> ";"
<play> ::= "play" <expression> ";" | "play" <expression> <expression> <expression> ";"    //Play all OR play from time X to time Y
<if>   ::= "if" <condition> "then" <statement>
<condition> ::= <expression> "==" <expression>
<expression> ::= <term> | <term> "+" <expression> | <term> "*" <expression>
<term> ::= <number> | <string> | <time> | <ID>
<string> ::= "\"" <filename> "\""
<number> ::= <integer>
<time>   ::= "\"" <integer> ":" <integer> "\""    //"Minutes:Seconds" position for time reference
<ID>     ::= <alphabetic string>

#  - Single-line comment: # <text> (until end of line)
## - Multi-line comment: ## <text> ## (multi-line, ends at next ##)
*/

//EXAMPLE
/*
DEPRECATED
frame "video.mp4" 5 to "frame5.bmp";             ¨ Extracts frame 5 as a bitmap.
concat "clip1.mp4" "clip2.mp4" to "output.mp4"; ¨ Concatenates two clips.
audio "video.mp4" 10 20 to "audio.mp3";         ¨ Extracts audio from 10s to 20s.
play "video.mp4";                               ¨ Plays the video.

UPDATED
frame "video.mp4" 13:23 to "frame5.bmp";          Extracts frame at 13 minutes and 23 seconds as a bitmap.
concat "clip1.mp4" "clip2.mp4" to "output.mp4"; ¨ Concatenates two clips.
audio "video.mp4" 0:10 0:20 to "audio.mp3";       Extracts audio from 10s to 20s.
play "video.mp4";                               ¨ Plays the video.
*/
