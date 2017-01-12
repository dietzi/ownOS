#include "includes.h"

char codeToChar(uint8_t);

char codeToChar(uint8_t keycode) {
	switch (keycode) {
		case 30:
			if(shift || shiftG) return (char)'A';
			else return (char)'a';
			break;
			
		case 48:
			if(shift || shiftG) return (char)'B';
			else return (char)'b';
			break;
			
		case 46:
			if(shift || shiftG) return (char)'C';
			else return (char)'c';
			break;
			
		case 32:
			if(shift || shiftG) return (char)'D';
			else return (char)'d';
			break;
			
		case 18:
			if(shift || shiftG) return (char)'E';
			else return (char)'e';
			break;
			
		case 33:
			if(shift || shiftG) return (char)'F';
			else return (char)'f';
			break;
			
		case 34:
			if(shift || shiftG) return (char)'G';
			else return (char)'g';
			break;
			
		case 35:
			if(shift || shiftG) return (char)'H';
			else return (char)'h';
			break;
			
		case 23:
			if(shift || shiftG) return (char)'I';
			else return (char)'i';
			break;
			
		case 36:
			if(shift || shiftG) return (char)'J';
			else return (char)'j';
			break;
			
		case 37:
			if(shift || shiftG) return (char)'K';
			else return (char)'k';
			break;
			
		case 38:
			if(shift || shiftG) return (char)'L';
			else return (char)'l';
			break;
			
		case 50:
			if(shift || shiftG) return (char)'M';
			else return (char)'m';
			break;
			
		case 49:
			if(shift || shiftG) return (char)'N';
			else return (char)'n';
			break;
			
		case 24:
			if(shift || shiftG) return (char)'O';
			else return (char)'o';
			break;
			
		case 25:
			if(shift || shiftG) return (char)'P';
			else return (char)'p';
			break;
			
		case 16:
			if(shift || shiftG) return (char)'Q';
			else return (char)'q';
			break;
			
		case 19:
			if(shift || shiftG) return (char)'R';
			else return (char)'r';
			break;
			
		case 31:
			if(shift || shiftG) return (char)'S';
			else return (char)'s';
			break;
			
		case 20:
			if(shift || shiftG) return (char)'T';
			else return (char)'t';
			break;
			
		case 22:
			if(shift || shiftG) return (char)'U';
			else return (char)'u';
			break;
			
		case 47:
			if(shift || shiftG) return (char)'V';
			else return (char)'v';
			break;
			
		case 17:
			if(shift || shiftG) return (char)'W';
			else return (char)'w';
			break;
			
		case 45:
			if(shift || shiftG) return (char)'X';
			else return (char)'x';
			break;
			
		case 44:
			if(shift || shiftG) return (char)'Y';
			else return (char)'y';
			break;
			
		case 21:
			if(shift || shiftG) return (char)'Z';
			else return (char)'z';
			break;
		
		case 57:
			return (char)' ';
			break;
			
		case 41:
			if(shift || shiftG) return (char)'°';
			else return (char)'^';
			break;
		
		case 2:
			if(shift || shiftG) return (char)'!';
			else return (char)'1';
			break;
		
		case 3:
			if(shift || shiftG) return (char)'"';
			else return (char)'2';
			break;
		
		case 4:
			if(shift || shiftG) return (char)'§';
			else return (char)'3';
			break;
		
		case 5:
			if(shift || shiftG) return (char)'$';
			else return (char)'4';
			break;
		
		case 6:
			if(shift || shiftG) return (char)'%';
			else return (char)'5';
			break;
		
		case 7:
			if(shift || shiftG) return (char)'&';
			else return (char)'6';
			break;
		
		case 8:
			if(shift || shiftG) return (char)'/';
			else return (char)'7';
			break;
		
		case 9:
			if(shift || shiftG) return (char)'(';
			else return (char)'8';
			break;
		
		case 10:
			if(shift || shiftG) return (char)')';
			else return (char)'9';
			break;
		
		case 11:
			if(shift || shiftG) return (char)'=';
			else return (char)'0';
			break;
				
		case 51:
			if(shift || shiftG) return (char)';';
			else return (char)',';
			break;
		
		case 52:
			if(shift || shiftG) return (char)':';
			else return (char)'.';
			break;
		
		case 53:
			if(shift || shiftG) return (char)'_';
			else return (char)'-';
			break;
		
		case 43:
			if(shift || shiftG) return (char)'\'';
			else return (char)'#';
			break;
		
		case 27:
			if(shift || shiftG) return (char)'*';
			else return (char)'+';
			break;
		
		case 12:
			if(shift || shiftG) return (char)'?';
			else return (char)'ß';
			break;
		
		case 13:
			if(shift || shiftG) return (char)'`';
			else return (char)'´';
			break;
		
		default:
			break;
	}
}