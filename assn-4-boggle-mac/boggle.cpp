/* File: boggle.cpp
 * ----------------
 * Author: Thabani Chibanda
 * Section leader: Ashwin Siripurapu
 */

#include "genlib.h"
#include "gboggle.h"
#include "sound.h"
#include "extgraph.h"
#include "simpio.h"
#include "lexicon.h"
#include "grid.h"
#include "vector.h"
#include "random.h"
#include <iostream>

//Functions implemented in main
void Welcome();
void SoundInstruct(string &check);
void BoardGridSetup(Grid<char> &board);
void UserTurn(playerT &user, Grid<char> &board, Lexicon &english, Lexicon &usedWords);
void CompTurn(playerT &comp, Grid<char> &board, Lexicon &english, Lexicon &usedWords);

//Recursive functions
bool WordExists(string &word, string &soFar, int &xPos, int &yPos, Lexicon &english, Grid<char> &board,
				Vector<string> &usedCubes);
string FindAllWords(string &word, Grid<char> &board, Vector<string> &usedCubes, Lexicon &english, Lexicon &usedWords,
					int &startX, int &startY);

//Helper functions used by other functions
void GiveInstructions();
bool Yes(string &input);
bool No(string &input);
void CompShuffle(string &cubeFaceUp);
void CheckWord(string &word, Lexicon &used, Lexicon &english);
bool CheckUsed(int &xPos, int &yPos, Vector<string> &usedCubes);
void Highlight(Vector<string> &usedCubes, int &xPos, int &yPos );
int Start(int &pos);
int End(int &pos);


const string STANDARD_CUBES[16] = {
  "AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS", 
  "AOOTTW", "CIMOTU", "DEILRX", "DELRVY", 
  "DISTTY", "EEGHNW", "EEINSU", "EHRTVW", 
  "EIOSST", "ELRTTY", "HIMNQU", "HLNNRZ"
};

const string BIG_BOGGLE_CUBES[25] = {
  "AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM", 
  "AEEGMU", "AEGMNN", "AFIRSY", "BJKQXZ", "CCNSTW", 
  "CEIILT", "CEILPT", "CEIPST", "DDLNOR", "DDHNOT", 
  "DHHLOR", "DHLNOR", "EIIITT", "EMOTTT", "ENSSSU", 
  "FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"
};


/**
 * Main function of the program
 *
 * @return integer 0
 */
int main() {
	
	InitGraphics();
	Randomize();
	
	Lexicon english("lexicon.dat"), usedWords;
	playerT user = Human, comp = Computer;
	Grid<char> board(4, 4);
	string input;
	
	Welcome();
	
	input = "sound";
	SoundInstruct(input);
	
	input = "instructions";
	SoundInstruct(input);
	
	DrawBoard(4, 4);
	BoardGridSetup(board);
	UserTurn(user, board, english, usedWords);
	CompTurn(comp, board, english, usedWords);
	
	return 0;
}

// **All functions implemented in main**


/**
 * Displays a welcome message to the user.
 *
 * @return void
 */
void Welcome() {
	
    cout << "Welcome!  You're about to play an intense game of mind-numbing Boggle." << endl
	<< "The good news is that you might improve your vocabulary a bit.  The" << endl
	<< "bad news is that you're probably going to lose miserably to this little" << endl
	<< "dictionary-toting hunk of silicon.  If only YOU had a gig of RAM...\n" << endl;
}

/**
 * Asks the user whether they would like sound or instructions depending on the
 * string passed along to the fucntion.
 *
 * @param string on what to ask for
 * @return void
 */
void SoundInstruct(string &check) {
	
	string input;
	
	cout << "Would you like " + check + "? ";
	input = GetLine();
	
	while ((!Yes(input)) && (!No(input))) {
		cout << "Invalid. Enter yes or no: ";
		input = GetLine();
	}
	
	if (Yes(input)) {
		if (check == "sound")
			SetSoundOn(true);
		else{
			PlayNamedSound("thats pathetic.wav");
			GiveInstructions();
		}
	}
	else if(No(input) && check == "sound") {
		SetSoundOn(false);
	}
	else {
		PlayNamedSound("yeah right.wav");
	}

}

/**
 * Sets up the visual board and the virtual grid used to keep track of the board.
 * The user has an option to set the board themselves with 16 characters of their
 * own choice. If they choose not to, the computer will randomly shuffle up the
 * and just create a board of its own.
 *
 * @param a grid of characters that keeps track of all face up characters
 * @return void
 */
void BoardGridSetup(Grid<char> &board){
	
	string userSet, cubeFaceUp;
	int count = 0;
	
	cout << "I'll give you a chance to set up the board to your specification." << endl
	<< "This makes it easier to confirm your boggle program is working." << endl
	<< "Do you want to force the board configuration? " << endl;
	userSet = GetLine();
	
	while ((!Yes(userSet)) && (!No(userSet))) {
		cout << "Invalid. Enter yes or no: ";
		userSet = GetLine();
	}
	
	if (Yes(userSet)) {
		
		cout << "Enter a 16-character string to identify which letters you want on the cubes." << endl
		<< "The first 4 letters are the cubes on the top row from left to right" << endl
		<< "next 4 letters are the second row, etc." << endl
		<< "Enter the string: " << endl;
		
		cubeFaceUp = GetLine();
		
		while ((cubeFaceUp.length() < 16) || (cubeFaceUp.length() > 16)) {
			cout << "String must include 16 characters. Try Again!";
			cubeFaceUp = GetLine();
		}
	}
	else {
		PlayNamedSound("dice rattle.wav");
		CompShuffle(cubeFaceUp);
		cubeFaceUp = ConvertToLowerCase(cubeFaceUp);
	}
	
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			LabelCube(i, j, cubeFaceUp[count]);
			board[i][j] = (cubeFaceUp[count]);
			count++;
		}
	}	
}

/**
 * Takes care of all the things that occur during the users turn. Prompts the user
 * for a word and then checks the board to see if that word exists on it. Players
 * turn ends when they enter an empty string
 *
 * @param the enumareted player tupe, a gird of all the boards characters, the english
 *		  dictionary, and a lexicon of all words used so far
 * @return void
 */
void UserTurn(playerT &user, Grid<char> &board, Lexicon &english, Lexicon &usedWords){
	
	cout << "Ok, take all the time you want and find all the words you can!" << endl
	<< "Signal that you're finished by entering an empty line." << endl
	<< "Enter word: ";
	
	while (true) {
		
		Vector<string> usedCubes;
		string word, soFar = "";
		int xPos = 0, yPos = 0;
		
		usedCubes.clear();
		word = GetLine();
		CheckWord(word, usedWords, english);
		
		if (word == "") break;
		
		if (WordExists(word, soFar, xPos, yPos, english, board, usedCubes)) {
			PlayNamedSound("Excellent.wav");
			cout << "Nice Find.\nEnter Word: ";
			usedWords.add(word);
			RecordWordForPlayer(word, user);
		}
		else{
			cout << "That word is not on the board. Nice try.\nEnter word: ";
			PlayNamedSound("not fooling anyone.wav");
		}
	}
}

/**
 * Function that handles the events of the computers turn.
 *
 * @param the enumarated computer type, grid of boards characters, all english
 *        words, and words taht have already been used
 * @return void
 */
void CompTurn(playerT &comp, Grid<char> &board, Lexicon &english, Lexicon &usedWords) {
	
	string word = "", newWord = "b";
	Vector<string> usedCubes;
	int i = 0, j = 0;
	
	PlayNamedSound("tweetle.wav");
	while (newWord != "") {
		newWord = FindAllWords(word, board, usedCubes, english, usedWords, i, j);
		usedWords.add(newWord);
		RecordWordForPlayer(newWord, comp);
	}
	
}

// **The two recursive functions**


/**
 * A recursive function that looks through the entire board looking for the word
 * that has been entered by the user. If the word exists then the function returns
 * true and highlights the word, if not then it returns false.
 *
 * @param users word, part of the word found so far, current x,y coordinates on the board
 *        being checked, all english words, grid of the boards characters, nd cubes used so far
 * @return true if the words is found, false otherwise
 */
bool WordExists(string &word, string &soFar, int &xPos, int &yPos, Lexicon &english, Grid<char> &board,
				Vector<string> &usedCubes) {
	
	if(soFar == word) {
		Highlight(usedCubes, xPos, yPos);
		return true;
	}
	else if(soFar == "") {
		
		for (int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				
				if (board[i][j] == word[0]){
					string newSoFar = soFar + board[i][j];
					
					string checker = IntegerToString(xPos) + "," + IntegerToString(yPos);
					usedCubes.add(checker);
					
					if(WordExists(word, newSoFar, i, j, english, board, usedCubes))
						return true;
				}
			}
		}
	}
	else if(soFar != "") {
		
		for (int i = Start(xPos); i <= End(xPos); i++) {
			for (int j = Start(yPos); j <= End(yPos); j++) {
				
				if(!(CheckUsed(i, j, usedCubes))){
					string newSoFar = soFar + board[i][j];
					Vector<string> newUsedCubes = usedCubes;
					string next = word.substr(0, newSoFar.length());
					
					string checker = IntegerToString(xPos) + "," + IntegerToString(yPos);
					newUsedCubes.add(checker);
                    
					if (newSoFar == next) {
						if(WordExists(word, newSoFar, i, j, english, board, newUsedCubes))
							return true;
					}
				}
			}
		}
	}
	
	return false;
}

/**
 * A recursive function that looks through the entire board looking for the words
 * that have not been found yet.
 *
 * @param part of the word found so far, grid of all boards characters, all the cubes used
 *        so far, lexicon of all english words, lexicon of words used so far, and starting points
 * @return a word that is in the english dictionary, at 4 letters long, and hasn't been found
 */
string FindAllWords(string &word, Grid<char> &board, Vector<string> &usedCubes, Lexicon &english, Lexicon &usedWords,
					int &startX, int &startY) {
	
	if ((english.containsWord(word)) && (!(usedWords.containsWord(word))) && (word.length() > 3)) {
		return word;
	}
	else if(word == "") {
		
		for (int i = startX; i < 4; i++) {
			for(int j = startY; j < 4; j++) {
				
				string newWord = word + board[i][j];
				
				string checker = IntegerToString(i) + "," + IntegerToString(j);
				usedCubes.add(checker);
				
				newWord = FindAllWords(newWord, board, usedCubes, english, usedWords, i, j);
				
				if (newWord.length() > 0)
					return newWord;
				else 
					usedCubes.clear();
			}
		}
	}
	else if(word != "") {
		
		for (int i = Start(startX); i <= End(startX); i++) {
			for (int j = Start(startY); j <= End(startY); j++) {
				
				if(!(CheckUsed(i, j, usedCubes))){
					
					string newWord = word + board[i][j];
					Vector<string> newUsedCubes = usedCubes;
					
					string checker = IntegerToString(i) + "," + IntegerToString(j);
					newUsedCubes.add(checker);
					
					if (english.containsPrefix(word)) {
						string newWord2 = FindAllWords(newWord, board, newUsedCubes, english, usedWords, i, j);
						if (newWord2.length() > 1)
							return newWord2;
					}
				}
			}
		}
	}
	
	return "";
}

// **All helper functions**


/**
 * Function that displays instructions on how to play the game
 *
 * @return void
 */
void GiveInstructions() {
	
    cout << endl << "The boggle board is a grid onto which I will randomly distribute" << endl
	<< "dice.  These 6-sided dice have letters rather than numbers on the faces, " << endl
	<< "creating a grid of letters on which you try to form words.  You go first, " << endl
	<< "entering the words you find that are formed by tracing adjoining " << endl
	<< "letters.  Two letters adjoin if they are next to each other horizontally, " << endl
	<< "vertically, or diagonally. A letter can only be used once in the word. Words" << endl
	<< "must be at least 4 letters long and can only be counted once.  You score points" << endl
	<< "based on word length, a 4-letter word is worth one, 5-letters two, etc.  After " << endl
	<< "your tiny brain is exhausted, I, the brilliant computer, will find all the " << endl
	<< "remaining words in the puzzle and double or triple your paltry score." << endl;
	
    cout << "\nHit return when you're ready...";
	
    GetLine();
}

/**
 * Function that checks a users input to see if they said or meant
 * to say yes
 *
 * @param users input
 * @return true if they said/meant yes, false otherwise
 */
bool Yes(string &input){
	
	if (input == "Yes")
		return true;
	else if (input == "yes")
		return true;
	else if (input == "YES")
		return true;
	else if (input == "Y")
		return true;
	else if (input == "y")
		return true;
	else if (input == "Yyes")
		return true;
	else if (input == "yyes")
		return true;
	else if (input == "Yess")
		return true;
	else if (input == "yess")
		return true;
	else
		return false;
	
}

/**
 * Function that checks a users input to see if they said or meant
 * to say no
 *
 * @param users input
 * @return true if they said/meant no, false otherwise
 */
bool No(string &input){
	
	if (input == "No")
		return true;
	else if (input == "no")
		return true;
	else if (input == "NO")
		return true;
	else if (input == "N")
		return true;
	else if (input == "n")
		return true;
	else if (input == "noo")
		return true;
	else if (input == "nno")
		return true;
	else
		return false;
	
}

/**
 * Function that shuffles up the boggle board in the instance that the user
 * would rather not do it themselves
 *
 * @param an empty string for the face up characters
 * @return void
 */
void CompShuffle(string &cubeFaceUp) {
	
	Vector<int> cubes;
	int select;
	
	for (int i = 0; i < 16; i++) {
		cubes.add(i);
	}
	
	for (int i = 0; i < 16; i++) {
		
		select = RandomInteger(0, cubes.size() - 1);
		string chosenCube = STANDARD_CUBES[select];
		cubes.removeAt(select);
		
		select = RandomInteger(0, chosenCube.length() - 1);
		
		cubeFaceUp += chosenCube[select];
	}
}

/**
 * Checks the word entered by the user to make sure it is long enough,
 * in the english dictionary, and hasn't already been found
 *
 * @param users word, words they have used, and all english words
 * @return void
 */
void CheckWord(string &word, Lexicon &usedWords, Lexicon &english) {
	
	while ((word.length() < 4) && (word != "")) {
		cout << "We have standards around here. That word isn't long enough.\nEnter Word: ";
		PlayNamedSound("not.wav");
		word = GetLine();
		CheckWord(word, usedWords, english);
	}
	
	while (!english.containsWord(word) && (word != "")) {
		cout << "That's not a word in the english dictionary. Don't be dumb.\nEnter Word: ";
		PlayNamedSound("denied.wav");
		word = GetLine();
		CheckWord(word, usedWords, english);
	}
	
	while (usedWords.containsWord(word) && (word != "")) {
		cout << "Already used that word. C'mon now.\nEnter Word: ";
		PlayNamedSound("yah as if.wav");
		word = GetLine();
		CheckWord(word, usedWords, english);
	}	
}

/**
 * Small helper function that checks if a cube has already been checked
 * during a search for a word
 *
 * @param x,y points on the grid of the cube being checked, and cubes used already
 * @return true if the cube was used or checked, false otherwise
 */
bool CheckUsed(int &xPos, int &yPos, Vector<string> &usedCubes) {
	
	string checker = IntegerToString(xPos) + "," + IntegerToString(yPos);
	
	for (int i = 0; i < usedCubes.size(); i++) {
		
		if (usedCubes[i] == checker)
			return true;
	}
	
	return false;
}

/**
 * Helper function that highlights the users word.
 *
 * @param cubes used in finding the word, (x,y) coordinates of the cube
 * @return void
 */
void Highlight(Vector<string> &usedCubes, int &xPos, int &yPos){
	
	string checker = IntegerToString(xPos) + "," + IntegerToString(yPos);
	usedCubes.add(checker);
	
	for (int i = 1; i < usedCubes.size(); i++) {
		string checker = usedCubes[i];
		int x = StringToInteger(checker.substr(0, 1));
		int y = StringToInteger(checker.substr(2, 1));
		
		HighlightCube(x, y, true);
	}
	
	Pause(1.5);
	
	for (int i = 1; i < usedCubes.size(); i++) {
		string checker = usedCubes[i];
		int x = StringToInteger(checker.substr(0, 1));
		int y = StringToInteger(checker.substr(2, 1));
		
		HighlightCube(x, y, false);
	}
}

/**
 * Function thats calculates the position to start the loop from when
 * beginning to check the area around a cube for the next word. Mainly
 * meant to make sure that the search doesn't go out of bounds
 *
 * @param center positon
 * @return lowest x or y position that can be checked
 */
int Start(int &pos) {
	
	if (pos == 0) {
		return 0;
	}
	else{
		return (pos - 1);
	}
}

/**
 * Function thats calculates the position to end the loop from when
 * beginning to check the area around a cube for the next word. Mainly
 * meant to make sure that the search doesn't go out of bounds
 *
 * @param center positon
 * @return highest x or y position that can be checked
 */
int End(int &pos) {
	
	if (pos == 3) {
		return 3;
	}
	else {
		return (pos + 1);
	}
}
