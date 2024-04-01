// 2021.11.19 
#include "float.h"

#ifdef WINDOWS
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "MyAI.h"

#define TIME_LIMIT 9.5

#define WIN 1.0
#define DRAW 0.2
#define LOSE 0.0
#define BONUS 0.3
#define BONUS_MAX_PIECE 8

#define OFFSET (WIN + BONUS)

#define NOEATFLIP_LIMIT 60
#define POSITION_REPETITION_LIMIT 3




const int Score_static[14] = { 1 , 0, 15  , 60 , 200 , 500 , 600 , 1 , 0,  15 , 60 , 200 , 500 , 600 };
const int Score_dynamic[10] = { 700 , 350 , 200 , 100 , 80 , 60 , 40 , 20 , 10 , 5};
const int Score_pawn[6] = {0 ,120 , 60 ,  40 , 30 , 24};   // less is  better
const int Score_pawn_my_dis[11] = {0 , 80 , 100 , 70 , 20, 10 , 0 ,0 ,0,0,0 }; 
const int Score_pawn_e_dis[11] = {0 , 150 , 100 , 70 , 20, 10 , 0 ,0 ,0,0,0 }; 
// real value > upper diff !!1
const int Score_pawn_global[6] = {0 , 2 , 3 , 5, 5, 6}; //more is better


int  Influence[7][7] ={
 {1,0,0,0,0,0,200},  //atk , def
 {0,0,0,0,0,0,0}, //gun
 {2,0,9,0,0,0,0},
 {2,0,10,53,0,0,0},
 {2,0,10,55,148,0,0},
 {2,0,10,55,150,178,0},
 {0,0,10,55,150,180,200},
};


const unsigned int Mask[32] = {
	0x00000012,
	0x00000025,
	0x0000004A,
	0x00000084,
	0x00000121,
	0x00000252,
	0x000004A4,
	0x00000848,
	0x00001210,
	0x00002520,
	0x00004A40,
	0x00008480,
	0x00012100,
	0x00025200,
	0x0004A400,
	0x00084800,
	0x00121000,
	0x00252000,
	0x004A4000,
	0x00848000,
	0x01210000,
	0x02520000,
	0x04A40000,
        0x08480000,
	0x12100000,
	0x25200000,
	0x4A400000,
	0x84800000,
	0x21000000,
	0x52000000,
	0xA4000000,
	0x48000000,

};

const int BitmapToInt[32]={
   31,0,1,5,2,16,27,6,3,14,17,19,28,11,7,21,30,4,15,26,13,18,10,20,29,25,12,9,24,8,23,22 
 
};


int BitHash( unsigned int x){
	return (x * 0x08ED2BE6) >> 27;
}









MyAI::MyAI(void){}

MyAI::~MyAI(void){}

bool MyAI::protocol_version(const char* data[], char* response){
	strcpy(response, "1.0.0");
  return 0;
}

bool MyAI::name(const char* data[], char* response){
	strcpy(response, "MyAI");
	return 0;
}

bool MyAI::version(const char* data[], char* response){
	strcpy(response, "1.0.0");
	return 0;
}

bool MyAI::known_command(const char* data[], char* response){
  for(int i = 0; i < COMMAND_NUM; i++){
		if(!strcmp(data[0], commands_name[i])){
			strcpy(response, "true");
			return 0;
		}
	}
	strcpy(response, "false");
	return 0;
}

bool MyAI::list_commands(const char* data[], char* response){
  for(int i = 0; i < COMMAND_NUM; i++){
		strcat(response, commands_name[i]);
		if(i < COMMAND_NUM - 1){
			strcat(response, "\n");
		}
	}
	return 0;
}

bool MyAI::quit(const char* data[], char* response){
  fprintf(stderr, "Bye\n"); 
	return 0;
}

bool MyAI::boardsize(const char* data[], char* response){
  fprintf(stderr, "BoardSize: %s x %s\n", data[0], data[1]); 
	return 0;
}

bool MyAI::reset_board(const char* data[], char* response){
	this->Red_Time = -1; // unknown
	this->Black_Time = -1; // unknown
	this->initBoardState();
	return 0;
}

bool MyAI::num_repetition(const char* data[], char* response){
  return 0;
}

bool MyAI::num_moves_to_draw(const char* data[], char* response){
  return 0;
}

bool MyAI::move(const char* data[], char* response){
  char move[6];
	sprintf(move, "%s-%s", data[0], data[1]);
	this->MakeMove(&(this->main_chessboard), move);
	return 0;
}

bool MyAI::flip(const char* data[], char* response){
  char move[6];
	sprintf(move, "%s(%s)", data[0], data[1]);
	this->MakeMove(&(this->main_chessboard), move);
	return 0;
}

bool MyAI::genmove(const char* data[], char* response){
	// set color
	if(!strcmp(data[0], "red")){
		this->Color = RED;
	}else if(!strcmp(data[0], "black")){
		this->Color = BLACK;
	}else{
		this->Color = 2;
	}
	// genmove
  char move[6];
	this->generateMove(move);
	sprintf(response, "%c%c %c%c", move[0], move[1], move[3], move[4]);
	return 0;
}

bool MyAI::game_over(const char* data[], char* response){
  fprintf(stderr, "Game Result: %s\n", data[0]); 
	return 0;
}

bool MyAI::ready(const char* data[], char* response){
  return 0;
}

bool MyAI::time_settings(const char* data[], char* response){
  return 0;
}

bool MyAI::time_left(const char* data[], char* response){
  if(!strcmp(data[0], "red")){
		sscanf(data[1], "%d", &(this->Red_Time));
	}else{
		sscanf(data[1], "%d", &(this->Black_Time));
	}
	fprintf(stderr, "Time Left(%s): %s\n", data[0], data[1]); 
	return 0;
}

bool MyAI::showboard(const char* data[], char* response){
  Pirnf_Chessboard();
	return 0;
}


// *********************** AI FUNCTION *********************** //

int MyAI::GetFin(char c)
{
	static const char skind[]={'-','K','G','M','R','N','C','P','X','k','g','m','r','n','c','p'};
	for(int f=0;f<16;f++)if(c==skind[f])return f;
	return -1;
}

int MyAI::ConvertChessNo(int input)
{
	switch(input)
	{
	case 0:
		return CHESS_EMPTY;
		break;
	case 8:
		return CHESS_COVER;
		break;
	case 1:
		return 6;
		break;
	case 2:
		return 5;
		break;
	case 3:
		return 4;
		break;
	case 4:
		return 3;
		break;
	case 5:
		return 2;
		break;
	case 6:
		return 1;
		break;
	case 7:
		return 0;
		break;
	case 9:
		return 13;
		break;
	case 10:
		return 12;
		break;
	case 11:
		return 11;
		break;
	case 12:
		return 10;
		break;
	case 13:
		return 9;
		break;
	case 14:
		return 8;
		break;
	case 15:
		return 7;
		break;
	}
	return -1;
}


void MyAI::initBoardState()
{	
	int iPieceCount[14] = {5, 2, 2, 2, 2, 2, 1, 5, 2, 2, 2, 2, 2, 1};
	memcpy(main_chessboard.CoverChess,iPieceCount,sizeof(int)*14);
	main_chessboard.Red_Chess_Num = 16;
	main_chessboard.Black_Chess_Num = 16;
	main_chessboard.NoEatFlip = 0;
	main_chessboard.HistoryCount = 0;

	//convert to my format
	int Index = 0;
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<4;j++)
		{
			main_chessboard.Board[Index] = CHESS_COVER;
			Index++;
		}
	}
	Pirnf_Chessboard();
}

void MyAI::generateMove(char move[6])
{	/* generateMove Call by reference: change src,dst */
	int StartPoint = 0;
	int EndPoint = 0;

	double t = -DBL_MAX;
#ifdef WINDOWS
	begin = clock();
#else
	gettimeofday(&begin, 0);
#endif
	
	// iterative-deeping, start from 3, time limit = <TIME_LIMIT> sec
	for(int depth = 3; !isTimeUp(); depth+=2){
		//if(depth > 3) break;
		this->node = 0;
		int best_move_tmp; double t_tmp;

		// run Nega-max
		t_tmp = alpha_beta_search(this->main_chessboard, &best_move_tmp, this->Color, 0, depth, -DBL_MAX , DBL_MAX);
		

		// check score
		// if search all nodes
		// replace the move and score
		if(!this->timeIsUp){
			t = t_tmp;
			StartPoint = best_move_tmp/100;
			EndPoint   = best_move_tmp%100;
			sprintf(move, "%c%c-%c%c",'a'+(StartPoint%4),'1'+(7-StartPoint/4),'a'+(EndPoint%4),'1'+(7-EndPoint/4));
		}
		// U: Undone
		// D: Done
		fprintf(stderr, "[%c] Depth: %2d, Node: %10d, Score: %+1.5lf, Move: %s\n", (this->timeIsUp ? 'U' : 'D'), 
			depth, node, t, move);
		fflush(stderr);
		

		if(t == 4 ){
			break;
		}
		// game finish !!!
		//if(t >= WIN){
		//	break;
		//}
	}
	
	char chess_Start[4]="";
	char chess_End[4]="";
	Pirnf_Chess(main_chessboard.Board[StartPoint],chess_Start);
	Pirnf_Chess(main_chessboard.Board[EndPoint],chess_End);
	printf("My result: \n--------------------------\n");
	printf("Nega max: %lf (node: %d)\n", t, this->node);
	printf("(%d) -> (%d)\n",StartPoint,EndPoint);
	printf("<%s> -> <%s>\n",chess_Start,chess_End);
	printf("move:%s\n",move);
	printf("--------------------------\n");
	this->Pirnf_Chessboard();

}

void MyAI::MakeMove(ChessBoard* chessboard, const int move, const int chess){
	int src = move/100, dst = move%100;
	if(src == dst){ // flip
		chessboard->Board[src] = chess;
		chessboard->CoverChess[chess]--;
		
		chessboard->NoEatFlip = 0;
		chessboard->pieceCount[chess] ++; 
	      

		chessboard->piece[chess]  |= (1 << dst);  //record position ver4	
		chessboard->occupied |= (1 << dst); // ver 4

	}else { // move
		int Dst = chessboard->Board[dst];
		if(Dst != CHESS_EMPTY){
			if(Dst / 7 == 0){ // red
			
				chessboard->Red_Chess_Num--;
			}

			else{ // black	
				
				
			
				chessboard->Black_Chess_Num--;
			}
			chessboard->NoEatFlip = 0;
			// =========== ver 3 ============
						
			int chess_no = chessboard->Board[dst] % 7 ;
			int color = chessboard->Board[dst] / 7 ;
				// enemy value up!  
			for(int i = 0 ; i<  chess_no ; i++){
				       if ( chess_no == 1 ) break;
					chessboard->upperCount[ (color^1)*7 + i  ] -- ;
			}

			
		chessboard->pieceCount[Dst] -= 1 ; 

			if(chessboard->pieceCount[color*7 + chess_no] == 0){
				chessboard->upperCount[ (color^1)*7 + chess_no  ] -- ;
			}
			
			// =============================
			(chessboard->piece[Dst])&=(~(1<<dst))  ; // remove the piece ver4
		}

		else{
			chessboard->NoEatFlip += 1;
		}
		
		int Src = chessboard->Board[src];
		(chessboard->occupied) |= (1 << dst);
  		(chessboard->occupied) &= (~(1<<src))  ; // remove the piece ver4

		
		

		chessboard->piece[Src] &= (~(1 << src));   // src piece to dst position!
		chessboard->piece[Src] |= (1 << dst);   // src piece to dst position!

		
		chessboard->Board[dst] = chessboard->Board[src];
		chessboard->Board[src] = CHESS_EMPTY;
	}
	chessboard->History[chessboard->HistoryCount++] = move;
}

void MyAI::MakeMove(ChessBoard* chessboard, const char move[6])
{ 
	int src, dst, m;
	src = ('8'-move[1])*4+(move[0]-'a');
	if(move[2]=='('){ // flip
		m = src*100 + src;
		printf("# call flip(): flip(%d,%d) = %d\n",src, src, GetFin(move[3])); 
	}else { // move
		dst = ('8'-move[4])*4+(move[3]-'a');
		m = src*100 + dst;
		printf("# call move(): move : %d-%d \n", src, dst);
	}
	MakeMove(chessboard, m, ConvertChessNo(GetFin(move[3])));
	Pirnf_Chessboard();
}

int MyAI::Expand(const int* board, const int color,int *Result ,const unsigned int * piece ,const unsigned int  occupied)
{
	// pawn, gun, hourse, car, elephant, advisor, general
	//  0     1     2      3      4          5       6        Red
	//  7     8     9     10      11        12      13        Black
	int ResultCount = 0;
	
	//ver 4 
	int enemy = (color^1)*7;
	for(int i= color*7+6 ; i>= color*7; i--){
		unsigned int p = piece[i];	
		while(p){ //make sure all this type are checked
			unsigned int mask = p & (-p); // get first position
			p ^= mask ;  // remove that position
			int src = BitmapToInt[BitHash(mask)] ; // bitmap -> int
			unsigned int dest = 0 ; // record which is legal
			
			
			if(i % 7 == 0){
				dest = Mask[src] & (piece[enemy+6] | piece[enemy]); 
			
			}

		//gun will not show here	
		
			else if(i%7 == 2){
				//the piece position which can kill !!
				dest = Mask[src] & (piece[enemy] | piece[enemy+2]); 
			}
		
			else if(i%7 == 3){
				dest = Mask[src] & (piece[enemy] | piece[enemy+2] | piece[enemy+3]); 
			
			}
		
		
			else if(i%7 == 4){
			
				dest = Mask[src] & (piece[enemy] | piece[enemy+2] | piece[enemy+3] | piece[enemy+4]); 
			}
		
		
			else if(i%7 == 5){
			
				dest = Mask[src] & (piece[enemy] | piece[enemy+2] | piece[enemy+3] | piece[enemy+4] | piece[enemy+5]); 
			}
		
			else if(i%7 == 6){
			
				dest = Mask[src] & (piece[enemy+6] | piece[enemy+2] | piece[enemy+3] | piece[enemy+4] | piece[enemy+5] ); 
			}
			
		
			// store information
			while(dest){
			  unsigned int ps = dest & (-dest);
			  dest ^= ps;
			  int result = BitmapToInt[BitHash(ps)];
			  
			  
			  
			  Result[ResultCount++] = src*100 + result;	
			}

		
		}
	
		
	
	
	}	
	
	for(int i= color*7+6 ; i>= color*7; i--){
		unsigned int p = piece[i];	
		while(p){ //make sure all this type are checked
			unsigned int mask = p & (-p); // get first position
			p ^= mask ;  // remove that position
			int src = BitmapToInt[BitHash(mask)] ; // bitmap -> int
			unsigned int dest = 0 ; // record which is legal
			dest = Mask[src] & (~occupied);
			

			while(dest){
			  unsigned int ps = dest & (-dest);
			  dest ^= ps;
			  int result = BitmapToInt[BitHash(ps)];
			  
			  
			  
			  Result[ResultCount++] = src*100 + result;	
			}


		
		}
	
	}

	
	
	return ResultCount;
}


// Referee
bool MyAI::Referee(const int* chess, const int from_location_no, const int to_location_no, const int UserId)
{
	// int MessageNo = 0;
	bool IsCurrent = true;
	int from_chess_no = chess[from_location_no];
	int to_chess_no = chess[to_location_no];
	int from_row = from_location_no / 4;
	int to_row = to_location_no / 4;
	int from_col = from_location_no % 4;
	int to_col = to_location_no % 4;

	if(from_chess_no < 0 || ( to_chess_no < 0 && to_chess_no != CHESS_EMPTY) )
	{  
		// MessageNo = 1;
		//strcat(Message,"**no chess can move**");
		//strcat(Message,"**can't move darkchess**");
		IsCurrent = false;
	}
	else if (from_chess_no >= 0 && from_chess_no / 7 != UserId)
	{
		// MessageNo = 2;
		//strcat(Message,"**not my chess**");
		IsCurrent = false;
	}
	else if((from_chess_no / 7 == to_chess_no / 7) && to_chess_no >= 0)
	{
		// MessageNo = 3;
		//strcat(Message,"**can't eat my self**");
		IsCurrent = false;
	}
	//check attack
	else if(to_chess_no == CHESS_EMPTY && abs(from_row-to_row) + abs(from_col-to_col)  == 1)//legal move
	{
		IsCurrent = true;
	}	
	else if(from_chess_no % 7 == 1 ) //judge gun
	{
		int row_gap = from_row-to_row;
		int col_gap = from_col-to_col;
		int between_Count = 0;
		//slant
		if(from_row-to_row == 0 || from_col-to_col == 0)
		{
			//row
			if(row_gap == 0) 
			{
				for(int i=1;i<abs(col_gap);i++)
				{
					int between_chess;
					if(col_gap>0)
						between_chess = chess[from_location_no-i] ;
					else
						between_chess = chess[from_location_no+i] ;
					if(between_chess != CHESS_EMPTY)
						between_Count++;
				}
			}
			//column
			else
			{
				for(int i=1;i<abs(row_gap);i++)
				{
					int between_chess;
					if(row_gap > 0)
						between_chess = chess[from_location_no-4*i] ;
					else
						between_chess = chess[from_location_no+4*i] ;
					if(between_chess != CHESS_EMPTY)
						between_Count++;
				}
				
			}
			
			if(between_Count != 1 )
			{
				// MessageNo = 4;
				//strcat(Message,"**gun can't eat opp without between one piece**");
				IsCurrent = false;
			}
			else if(to_chess_no == CHESS_EMPTY)
			{
				// MessageNo = 5;
				//strcat(Message,"**gun can't eat opp without between one piece**");
				IsCurrent = false;
			}
		}
		//slide
		else
		{
			// MessageNo = 6;
			//strcat(Message,"**cant slide**");
			IsCurrent = false;
		}
	}
	else // non gun
	{
		//judge pawn or king

		//distance
		if( abs(from_row-to_row) + abs(from_col-to_col)  > 1)
		{
			// MessageNo = 7;
			//strcat(Message,"**cant eat**");
			IsCurrent = false;
		}
		//judge pawn
		else if(from_chess_no % 7 == 0)
		{
			if(to_chess_no % 7 != 0 && to_chess_no % 7 != 6)
			{
				// MessageNo = 8;
				//strcat(Message,"**pawn only eat pawn and king**");
				IsCurrent = false;
			}
		}
		//judge king
		else if(from_chess_no % 7 == 6 && to_chess_no % 7 == 0)
		{
			// MessageNo = 9;
			//strcat(Message,"**king can't eat pawn**");
			IsCurrent = false;
		}
		else if(from_chess_no % 7 < to_chess_no% 7)
		{
			// MessageNo = 10;
			//strcat(Message,"**cant eat**");
			IsCurrent = false;
		}
	}
	return IsCurrent;
}

// always use my point of view, so use this->Color
double MyAI::Evaluate(const ChessBoard* chessboard, 
	const int legal_move_count, const int color , const int depth , const int maxdepth){
	// score = My Score - Opponent's Score
	// offset = <WIN + BONUS> to let score always not less than zero
	double offset = 1.5;
	double score = offset;
	//bool finish;
	/*
	if(legal_move_count == 0){ // Win, Lose
		if(color == this->Color){ // Lose
			score += LOSE - WIN;
		}else{ // Win
			score += WIN - LOSE;
		}
		finish = true;
	}else if(isDraw(chessboard)){ // Draw
		// score += DRAW - DRAW;
		finish = true;
	}else{*/ // no conclusion
		// static material values
		// cover and empty are both zero
		
		// pawn, gun, hourse, car, elephant, offical, general
	        //  0     1     2      3      4          5       6        Red
		//  7     8     9     10      11        12      13        Black
		int mine = this->Color*7+6;
		int e = (this->Color^1)*7+6 ;

		double piece_value = 0;
		
		int num_enemy = 0;
		if(this->Color == 1){
			num_enemy = chessboard->Red_Chess_Num;
		}
		else{
		
			num_enemy = chessboard->Black_Chess_Num;
		}
	//	printf("\n%d",num_enemy);
		if(num_enemy == 0){
			return 4;
		}
		unsigned int king_m  = chessboard->piece[mine];
		int m_king_pos = -1;
		if(king_m)  m_king_pos = BitmapToInt[BitHash(king_m)] ; 
		

		
		unsigned int king_e  = chessboard->piece[e];
		int e_king_pos = -1;
		if(king_e) e_king_pos =  BitmapToInt[BitHash(king_e)] ; 
		

		int my_king_col = m_king_pos %4;
		int my_king_row = m_king_pos /4;

		int e_king_col = e_king_pos %4;
		int e_king_row = e_king_pos /4;

		for(int i= e; i> e-7; i--){
				// chess == i
				unsigned int p = chessboard->piece[i];	
				while(p){ //make sure all this type are checked
					unsigned int mask = p & (-p); // get first position
					p ^= mask ;  // remove that position
					int src = BitmapToInt[BitHash(mask)] ; // bitmap -> int
					
					piece_value -= Score_static[i];
							
					
					if(chessboard->pieceCount[mine]==1 && i == e-6){
						piece_value -= Score_pawn_e_dis[10-abs(my_king_col-src%4)-abs(my_king_row - src/4)]; //pawn pos
						piece_value -= Score_pawn[chessboard->pieceCount[i]];
					}	
					piece_value -= Score_dynamic[chessboard->upperCount[i]]; // upper count
					
				
						
					

				}
				
		
		}


		if(chessboard->pieceCount[mine] == 0){
			piece_value -= 100;
		}

		piece_value -= Score_pawn_global[chessboard->pieceCount[(this->Color^1)*7]];
		for(int i= mine; i> mine-7; i--){
				// chess == i , mine is my king no
				unsigned int p = chessboard->piece[i];	
				while(p){//make sure all this type are checked
					unsigned int mask = p & (-p); // get first position
					p ^= mask ;  // remove that position
					int src = BitmapToInt[BitHash(mask)] ; // bitmap -> int
					

							



					piece_value += Score_static[i];
							
					

					if(chessboard->pieceCount[e]==1 && i == mine-6){

							piece_value += Score_pawn[chessboard->pieceCount[i]];
							piece_value += Score_pawn_my_dis[(10-abs(e_king_col-src%4) - abs(e_king_row - src/4))]; //pawn pos
						
					}	
										
						// let piece closer to enemy
						if( num_enemy < 5 || chessboard->NoEatFlip>=20){	
							for(int k = e ;  k> e-7 ; k--){
								unsigned int q = chessboard->piece[k];	
								while(q){ //make sure all this type are checked
									unsigned int q_mask = q & (-q); // get first position
									q ^= q_mask ;  // remove that position
									int dst = BitmapToInt[BitHash(q_mask)] ; // bitmap -> int
								
									int dis = (abs( src/4 - dst/4 ) + abs( src%4 - dst%4)  ) ;
									double mul = (dis == 1)? 0.5:pow(2.0, 2.0-dis);
		
									//printf("\n%d %d" , src , dst);
									piece_value += int(Influence[i%7][k%7] * mul *1.3) ; //encourage chase
									
									if(abs(src/4 - dst/4) == abs(src%4 - dst%4) && abs(src/4-dst/4) == 1){
										piece_value += Influence[i%7][k%7]*0.1;
									}

								}
							
								}
						}
						piece_value += Score_dynamic[chessboard->upperCount[i]]; // upper count
				}
				
				
		
		}
		if(chessboard->pieceCount[e] == 0){
				piece_value += 100;
		}
	
		piece_value += Score_pawn_global[chessboard->pieceCount[this->Color*7]];
		
			  

	//	int idx = chessboard->HistoryCount-1 ; 	
	//	if( (chessboard->History[idx] == chessboard->History[idx-4] && chessboard->History[idx] == chessboard->History[idx-8] ) || chessboard->NoEatFlip>=55){
			
			// avoid draw
	//		piece_value -= 10000;
	//	}
	//	piece_value += rand()%50;
		double total =  8000;
		

		piece_value = piece_value / total  * (WIN - 0.01);
		score += piece_value; 
	//	finish = false;
	//}

	// Bonus (Only Win / Draw)
	//
	/*
	if(finish){
		if((this->Color == RED && chessboard->Red_Chess_Num > chessboard->Black_Chess_Num) ||
			(this->Color == BLACK && chessboard->Red_Chess_Num < chessboard->Black_Chess_Num)){
			if(!(legal_move_count == 0 && color == this->Color)){ // except Lose
				double bonus = BONUS / BONUS_MAX_PIECE * 
					abs(chessboard->Red_Chess_Num - chessboard->Black_Chess_Num);
				if(bonus > BONUS){ bonus = BONUS; } // limit
				score += bonus;
			}
		}else if((this->Color == RED && chessboard->Red_Chess_Num < chessboard->Black_Chess_Num) ||
			(this->Color == BLACK && chessboard->Red_Chess_Num > chessboard->Black_Chess_Num)){
			if(!(legal_move_count == 0 && color != this->Color)){ // except Lose
				double bonus = BONUS / BONUS_MAX_PIECE * 
					abs(chessboard->Red_Chess_Num - chessboard->Black_Chess_Num);
				if(bonus > BONUS){ bonus = BONUS; } // limit
				score -= bonus;
			}
		}
	}*/
	return score ;
}

double MyAI::alpha_beta_search(const ChessBoard chessboard, int* move, const int color, const int depth, const int remain_depth, double alpha, double beta){

	int Moves[2048];
	int move_count = 0;

	// move
	// check every possible move and store them in Moves array
	move_count = Expand(chessboard.Board, color, Moves , chessboard.piece , chessboard.occupied);
	
	if(isTimeUp() || // time is up
		remain_depth == 0 || // reach limit of depth
		chessboard.Red_Chess_Num == 0 || // terminal node (no chess type)
		chessboard.Black_Chess_Num == 0 || // terminal node (no chess type)
		move_count == 0 // terminal node (no move type)
		){
		this->node++;
		// odd: *-1, even: *1
		return Evaluate(&chessboard, move_count, color , depth-remain_depth , depth)  * (depth&1 ? -1 : 1);
	}else{
		double m = alpha;
		int new_move;
		// search deeper
		//

		//printf("%d\n",move_count);
		//
		for(int i = 0; i < move_count; i++){ // move
			ChessBoard new_chessboard = chessboard; // shallow?
			MakeMove(&new_chessboard, Moves[i], 0); // 0: dummy
			// move every possible move
			double t = -alpha_beta_search(new_chessboard, &new_move, color^1, depth+1, remain_depth-1, -beta, -m);
			
			if(isDraw(&new_chessboard)){
				t *= 0.2;
				//t-=200;
			}

		//	if (depth == 0 || depth == 1 ) printf("\n%d %f %d :)\n" ,remain_depth ,  t , Moves[i] );
			if(  t > m){
				//if(depth == 0 || depth == 1 ) 
	//			printf("\n%d %f %d\n" ,remain_depth ,  t , Moves[i] );
				m = t;
				if (m >= beta) {
					return m;
				}
				*move = Moves[i];
			}
		}
		return m;
	}
}

bool MyAI::isDraw(const ChessBoard* chessboard){
	// No Eat Flip
	if(chessboard->NoEatFlip >= NOEATFLIP_LIMIT){
		return true;
	}

	// Position Repetition
	int last_idx = chessboard->HistoryCount - 1;
	// -2: my previous ply
	int idx = last_idx - 2;
	// All ply must be move type
	int smallest_repetition_idx = last_idx - (chessboard->NoEatFlip / POSITION_REPETITION_LIMIT);
	// check loop
	while(idx >= 0 && idx >= smallest_repetition_idx){
		if(chessboard->History[idx] == chessboard->History[last_idx]){
			// how much ply compose one repetition
			int repetition_size = last_idx - idx;
			bool isEqual = true;
			for(int i = 1; i < POSITION_REPETITION_LIMIT && isEqual; ++i){
				for(int j = 0; j < repetition_size; ++j){
					int src_idx = last_idx - j;
					int checked_idx = last_idx - i*repetition_size - j;
					if(chessboard->History[src_idx] != chessboard->History[checked_idx]){
						isEqual = false;
						break;
					}
				}
			}
			if(isEqual){
				return true;
			}
		}
		idx -= 2;
	}

	return false;
}

bool MyAI::isTimeUp(){
	double elapsed; // ms
	
	// design for different os
#ifdef WINDOWS
	clock_t end = clock();
	elapsed = (end - begin);
#else
	struct timeval end;
	gettimeofday(&end, 0);
	long seconds = end.tv_sec - begin.tv_sec;
	long microseconds = end.tv_usec - begin.tv_usec;
	elapsed = (seconds*1000 + microseconds*1e-3);
#endif

	this->timeIsUp = (elapsed >= TIME_LIMIT*1000);
	return this->timeIsUp;
}

//Display chess board
void MyAI::Pirnf_Chessboard()
{	
	char Mes[1024] = "";
	char temp[1024];
	char myColor[10] = "";
	if(Color == -99)
		strcpy(myColor, "Unknown");
	else if(this->Color == RED)
		strcpy(myColor, "Red");
	else
		strcpy(myColor, "Black");
	sprintf(temp, "------------%s-------------\n", myColor);
	strcat(Mes, temp);
	strcat(Mes, "<8> ");
	for(int i = 0; i < 32; i++){
		if(i != 0 && i % 4 == 0){
			sprintf(temp, "\n<%d> ", 8-(i/4));
			strcat(Mes, temp);
		}
		char chess_name[4] = "";
		Pirnf_Chess(this->main_chessboard.Board[i], chess_name);
		sprintf(temp,"%5s", chess_name);
		strcat(Mes, temp);
	}
	strcat(Mes, "\n\n     ");
	for(int i = 0; i < 4; i++){
		sprintf(temp, " <%c> ", 'a'+i);
		strcat(Mes, temp);
	}
	strcat(Mes, "\n\n");
	printf("%s", Mes);
}


// Print chess
void MyAI::Pirnf_Chess(int chess_no,char *Result){
	// XX -> Empty
	if(chess_no == CHESS_EMPTY){	
		strcat(Result, " - ");
		return;
	}
	// OO -> DarkChess
	else if(chess_no == CHESS_COVER){
		strcat(Result, " X ");
		return;
	}

	switch(chess_no){
		case 0:
			strcat(Result, " P ");
			break;
		case 1:
			strcat(Result, " C ");
			break;
		case 2:
			strcat(Result, " N ");
			break;
		case 3:
			strcat(Result, " R ");
			break;
		case 4:
			strcat(Result, " M ");
			break;
		case 5:
			strcat(Result, " G ");
			break;
		case 6:
			strcat(Result, " K ");
			break;
		case 7:
			strcat(Result, " p ");
			break;
		case 8:
			strcat(Result, " c ");
			break;
		case 9:
			strcat(Result, " n ");
			break;
		case 10:
			strcat(Result, " r ");
			break;
		case 11:
			strcat(Result, " m ");
			break;
		case 12:
			strcat(Result, " g ");
			break;
		case 13:
			strcat(Result, " k ");
			break;
	}
}


