﻿#include "bitboard72.hpp"
#include <algorithm>
#include <chrono>
#include <vector>
#include <fstream>

/*Links that helped me : 
http://www.informatik.uni-trier.de/~fernau/DSL0607/Masterthesis-Viergewinnt.pdf
http://fabpedigree.com/james/connect4.htm
https://roadtolarissa.com/connect-4-ai-how-it-works/
https://towardsdatascience.com/creating-the-perfect-connect-four-ai-bot-c165115557b0
http://blog.gamesolver.org/
https://www.chessprogramming.org/Main_Page
*/


class Connect4AI
{
public:
	Connect4AI(std::string _filename = "trash.txt", std::vector<uint8_t> _moveOrder = {6,5,7,4,8,3,9,2,10,1,0,11}, int _depth = 5) : m_board_AI(0, 0), m_board_Opp(0, 0), m_filename(_filename), m_moveOrder(_moveOrder), m_depth(_depth) {	}
	//AI vs Human
	void game()
	{
		bitboard72 board_AI(0, 0);
		bitboard72 board_Opp(0, 0);
		std::wcout << L"Voulez vous commencer ? [y/n]";
		wchar_t choiceStart;
		std::wcin >> choiceStart;
		if (choiceStart == 'n')
		{
			board_AI = results(board_AI, bestMove(board_AI, board_Opp));
		}
		int choice = -1;
		displayGameConsole(board_AI, board_Opp);
		while (!isTerminal(board_AI) && !isTerminal(board_Opp))
		{
			do
			{
				std::wcout << L"Saisissez un coup : ";
				std::wcin >> choice;
			} while (choice < 0 || choice >11);
			board_Opp = results(board_Opp, choice);
			displayGameConsole(board_AI, board_Opp);
			board_AI = results(board_AI, bestMove(board_AI, board_Opp));
			displayGameConsole(board_AI, board_Opp);
		}
	}
	//AI vs AI (vs itself)
	void gameAlone()
	{
		bitboard72 board_AI(0, 0);
		bitboard72 board_Opp(0, 0);
		uint8_t move;
		char winner = '.';
		while (!isTerminal(board_AI) && !isTerminal(board_Opp))
		{
			move = bestMove(board_Opp, board_AI);
			m_movesSequences.push_back(move);
			board_Opp = results(board_Opp, move);
			//displayGameConsole(board_AI, board_Opp);
			if (isTerminal(board_Opp)) {
				//std::wcout << "X Won";
				winner = 'X';
				break;
			}
			move = bestMove(board_AI, board_Opp);
			m_movesSequences.push_back(move);
			board_AI = results(board_AI, move);
			//displayGameConsole(board_AI, board_Opp);
			if (isTerminal(board_AI)) {
				//std::wcout << "O Won";
				winner = 'O';
				break;
			}
		}
		savePartyData(winner);
	}

private:
	//--------------------VARIABLES--------------------------
	bitboard72 m_board_AI;//Bitboard containing the move of the AI
	bitboard72 m_board_Opp;//Bitboard containing the move of the opponent
	uint8_t m_possibleCoord[12] = { 5,5,5,5,5,5,5,5,5,5,5,5 };//We store the possible coordinates by row so we don't need to calculate each time we want to make move
	std::vector<uint8_t> m_moveOrder;
	std::vector<int> m_times;
	std::chrono::time_point<std::chrono::system_clock>m_start, m_end;
	std::string m_filename;
	std::vector<uint8_t> m_movesSequences;
	int m_depth;

	//--------------------METHODS--------------------------
	//Return a bitboard equal to board with a 1 in the highest coordinates of the column *col*
	bitboard72 results(bitboard72 board, uint8_t col)
	{
		uint8_t temp = m_possibleCoord[col];
		m_possibleCoord[col]--;
		return setCellState(board, temp, col);
	}
	//Return a value corresponding to the quality of the board for the player which own the *baord1*. I'm not sure if the depth really changes something, I'll try some test later
	int heuristic(bitboard72 board1,bitboard72 board2,int depth)
	{	
		if (isTerminal(board1))
			return 10000 + depth;
		if (isTerminal(board2))
			return -10000 + (m_depth-depth);
		else
		{
			return hasFollow3(board1, board2) * 5 + hasFollow2(board1, board2);
		}

	}
	//Check if *board* has four 1 in a row,column or diagonal
	bool isTerminal(bitboard72 board)
	{
		int directions[4] = { 1,13,12,14 };
		for (int i = 0; i < 4; i++)
		{
			if (board & board >> directions[i] & board >> directions[i] * 2 &  board>>directions[i] * 3)
				return true;
		}
		return false;
	}
	//The two following methods are kind of messy. They search the number of sequences that aren't blocked by the opponent. 
	//Return the number of good three 1 in a row,column and diagonal
	int hasFollow3(bitboard72 board1,bitboard72 board2)
	{
		int directions[4] = { 1,13,12,14 };
		int count = 0;
		bitboard128 preventBord(0xFFFFFFFFFFFFE001, 0x8004002001000);//This bitboard prevent for value not in the int he board to be count has valid
		for (int i = 0; i < 4; i++)
		{
			bitboard72 test1 = ~board2 & (board1 >> directions[i])& (board1 >> directions[i] * 2)& (board1 >> directions[i] * 3) & ~preventBord;                   // |   | O | O | O |
			bitboard72 test2 = board1 & (~(board2 >> directions[i]))& (board1 >> directions[i] * 2)& (board1 >> directions[i] * 3)& ~(preventBord>>directions[i]);  // | O |   | O | O |
			bitboard72 test3 = board1 & (board1 >> directions[i])& (~(board2 >> directions[i] * 2))& (board1 >> directions[i] * 3)& ~(preventBord>>directions[i]*2);// | O | O |   | O |
			bitboard72 test4 = board1 & (board1 >> directions[i])& (board1 >> directions[i] * 2)& (~(board2 >> directions[i] * 3))& ~(preventBord>>directions[i]*3);// | O | O | O |   |
			count += countCell(test1 | test2 | test3 | test4);
			/*if (board1 & (board1 >> directions[i])& (board1 >> directions[i] * 2)^ (board2 >> directions[i] * 3))
				return true;
			if (board2 ^ (board1 >> directions[i])& (board1 >> directions[i] * 2)& (board1 >> directions[i] * 3))
				return true;*/
		}
		return count;
	}
	//Return the number of good two 1 in a row, column, and diagonal
	int hasFollow2(bitboard72 board1, bitboard72 board2)
	{
		int directions[4] = { 1,13,12,14 };
		int count = 0;
		bitboard128 preventBord(0xFFFFFFFFFFFFE001, 0x8004002001000);//This bitboard prevent for value not in the int he board to be count has valid
		for (int i = 0; i < 4; i++)
		{
				bitboard72 test1 = (board1 & (board1 >> directions[i]))& (~((board2 >> directions[i] * 2) | (board2 >> directions[i] * 3))) & ~((preventBord >> directions[i] * 2) | (preventBord >> directions[i] * 3));   // | O | O |   |   |
				bitboard72 test2 = ((board1 >> directions[i])& (board1 >> directions[i] * 2))& (~((board2) | (board2 >> directions[i] * 3))) & ~((preventBord) | (preventBord >> directions[i] * 3));						// |   | O | O |   |
				bitboard72 test3 = (~(board2 | (board2 >> directions[i])))& ((board1 >> directions[i] * 2)& (board1 >> directions[i] * 3)) & ~((preventBord) | (preventBord >> directions[i]));                             // |   |   | O | O |
				bitboard72 test4 = ((board1 & (board1 >> directions[i] * 3))& (~((board2 >> directions[i]) | (board2 << directions[i] * 2)))) & ~((preventBord >> directions[i]) | (preventBord >> directions[i] * 2));     // | O |   |   | O |
				bitboard72 test5 = ((board1 >> directions[i])& (board1 >> directions[i] * 3))& (~(board2 | (board2 >> directions[i] * 2))) & ~((preventBord) | (preventBord >> directions[i] * 2));						    // | O |   | O |   |
				bitboard72 test6 = ((board1) & (board1 >> directions[i] * 2))& (~((board2 >> directions[i]) | (board2 >> directions[i] * 3))) & ~((preventBord >> directions[i]) | (preventBord >> directions[i] * 3));		// |   | O |   | O |
				count += countCell(test1 | test2 | test3 | test4 | test5 | test6);
		}
		return count;
	}
	//The minmax algorithm
	int minmax(bitboard72 board_AI, bitboard72 board_Opp, int depth, int alpha, int beta, bool player)
	{
		//displayConsole(to_wstring(board_AI | board_Opp));
		if (depth == 0 || isTerminal(board_AI) || isTerminal(board_Opp) || countCell(board_AI | board_Opp)>=42)
			return heuristic(board_AI, board_Opp,depth) - heuristic(board_Opp,board_AI,depth);

		if (player)
		{
			int value = -100000;
			uint16_t childs = getRow(board_AI | board_Opp, 0);
			for (uint8_t i = 0; i < 12; i++)
			{
				if (!((1 << m_moveOrder[i]) & childs))
				{
					value = std::max(value, minmax(results(board_AI, m_moveOrder[i]), board_Opp, depth - 1, alpha, beta, !player));
					m_possibleCoord[m_moveOrder[i]]++;
					if (value >= beta)
						return value;
					alpha = std::max(alpha, value);
				}
			}
			return value;
		}
		else
		{
			int value = 100000;
			uint16_t childs = getRow(board_AI | board_Opp, 0);
			for (uint8_t i = 0; i < 12; i++)
			{
				if (!((1 << m_moveOrder[i]) & childs))
				{
					value = std::min(value, minmax(board_AI, results(board_Opp, m_moveOrder[i]), depth - 1, alpha, beta, !player));
					m_possibleCoord[m_moveOrder[i]]++;
					if (value <= alpha)
						return value;
					beta = std::min(beta, value);
				}
			}
			return value;
		}



	}
	//Return the best move for the player who own *board_AI* against the player who owns *board_opp*
	uint8_t bestMove(bitboard72 board_AI, bitboard72 board_Opp)
	{
		m_start = std::chrono::system_clock::now();
;		int bestMove = -1, bestValue = -100000, depth = m_depth;
		uint16_t childs = getRow(board_AI | board_Opp, 0);
		for (int i = 0; i < 12; i++)
		{
			if (!((1 << m_moveOrder[i]) & childs))
			{
				int value = minmax(results(board_AI, m_moveOrder[i]), board_Opp, depth - 1, -10000, 10000, false);
				//std::wcout << L"Move : " << moveOrder[i] << L" Value " << value << std::endl;
				m_possibleCoord[m_moveOrder[i]]++;
				if (value > bestValue)
				{
					bestMove = m_moveOrder[i];
					bestValue = value;
				}
			}
		}
		m_end = std::chrono::system_clock::now();
		m_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(m_end - m_start).count());
		return bestMove;
	}
	//Save times, moves and winner of the party in a txt/csv file
	void savePartyData(char winner)
	{
		std::ofstream wStream(m_filename.c_str(), std::ios::app);
		if (wStream)
		{
			for(int i = 0;i<m_times.size();i++)
			{
				wStream << std::to_string(m_times[i]) << ';';
			}
			wStream << std::endl;
			for(int i = 0;i<m_movesSequences.size();i++)
			{
				wStream << std::to_string(m_movesSequences[i]) << ';';
			}
			wStream << std::endl << winner << ';' << std::endl;
		}
	}
	//Display the game in the console. AI = O, Opponent = X
	void displayGameConsole(bitboard72 board_AI, bitboard72 board_Opp)
	{
		std::wstring str = L" 0  1  2  3  4  5  6  7  8  9  10 11\n";
		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				if (getCellState(board_AI, i, j))
					str += L" O ";
				else if (getCellState(board_Opp, i, j))
					str += L" X ";
				else
					str += L" . ";
			}
			str += L"\n";
		}
		str += L"\n";
		displayConsole(str);
	}
};




