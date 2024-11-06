#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
#include<pthread.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_ttf.h>
using namespace std;

int windowHeight;
int windowWidth;

//Following two functions print Start Screen with game name:
void StartText(SDL_Renderer* renderer, TTF_Font* font)
{
	SDL_Color color = { 33, 46, 150, 255 }; // Blue-ish
	SDL_Surface* surface = TTF_RenderText_Solid(font, "PING PONG", color); //game name
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	int textWidth = surface->w;
	int textHeight = surface->h;
	SDL_Rect textBox = { ((windowWidth - textWidth) / 2) - 525, ((windowHeight - textHeight) / 2) - 350, 1200, 700 }; //printing in the centre

	SDL_RenderCopy(renderer, texture, NULL, &textBox);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

bool StartGame(SDL_Renderer* renderer, TTF_Font* font)
{
	SDL_Event E;
	bool startGame = false;
	while (!startGame)
	{
		while (SDL_PollEvent(&E))
		{
			if (E.type == SDL_KEYDOWN && E.key.keysym.sym == SDLK_RETURN)
			{
				startGame = true;
			}
		}

		SDL_SetRenderDrawColor(renderer, 5, 0, 0, 255);
		SDL_RenderClear(renderer);
		StartText(renderer, font);
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}
	return true;
}

int rightScore = 0;
int leftScore = 0;
SDL_Rect rightPaddle = { 0,0,30,300 };
SDL_Rect leftPaddle = { 0,0,30,300 };
bool GameOver = false;
int xBall;
int yBall;
int ballRadius = 15;

void drawingBall(SDL_Renderer* renderer, int x, int y, int r, int thickness)
{
	for (int t = 0; t < thickness; t++)
	{
		int d = 3 - 2 * (r + t);
		int i = 0, j = r + t;

		while (i <= j)
		{
			SDL_RenderDrawPoint(renderer, x + i, y + j);
			SDL_RenderDrawPoint(renderer, x - i, y + j);
			SDL_RenderDrawPoint(renderer, x + i, y - j);
			SDL_RenderDrawPoint(renderer, x - i, y - j);
			SDL_RenderDrawPoint(renderer, x + j, y + i);
			SDL_RenderDrawPoint(renderer, x - j, y + i);
			SDL_RenderDrawPoint(renderer, x + j, y - i);
			SDL_RenderDrawPoint(renderer, x - j, y - i);

			if (d < 0)
				d += 4 * i + 6;
			else
			{
				d += 4 * (i - j) + 10;
				j--;
			}
			i++;
		}
	}

}

void ScorePrinting(SDL_Renderer* renderer, TTF_Font* f, int score, int xCentre, int yCentre)
{
	SDL_Color color = { 0,0,0,255 }; //black text
	SDL_Surface* surface = TTF_RenderText_Solid(f, to_string(score).c_str(), color);
	SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect textBox = { xCentre,yCentre,80,130 };
	SDL_RenderCopy(renderer, text, nullptr, &textBox);
	SDL_DestroyTexture(text);
	SDL_FreeSurface(surface);
}

void Layout(SDL_Renderer* renderer, TTF_Font* f)
{
	//background color:
	SDL_Rect screen = { 0,0,windowWidth,windowHeight };
	SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); //grey
	SDL_RenderFillRect(renderer, &screen);

	//printing of borders:
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_Rect topHorizontal;
	topHorizontal.x = 0;
	topHorizontal.y = 0;
	topHorizontal.w = windowWidth;
	topHorizontal.h = 5;
	SDL_RenderFillRect(renderer, &topHorizontal);

	SDL_Rect bottomHorizontal;
	bottomHorizontal.x = 0;
	bottomHorizontal.y = windowHeight - 5;
	bottomHorizontal.w = windowWidth;
	bottomHorizontal.h = 5;
	SDL_RenderFillRect(renderer, &bottomHorizontal);

	SDL_Rect middleHorizontal;
	middleHorizontal.x = 0;
	middleHorizontal.y = 140;
	middleHorizontal.w = windowWidth;
	middleHorizontal.h = 10;
	SDL_RenderFillRect(renderer, &middleHorizontal);

	SDL_Rect leftVerticle;
	leftVerticle.x = 0;
	leftVerticle.y = 0;
	leftVerticle.w = 10;
	leftVerticle.h = windowHeight;
	SDL_RenderFillRect(renderer, &leftVerticle);

	SDL_Rect scoreDivider;
	scoreDivider.x = (windowWidth / 2) - 10;
	scoreDivider.y = 0;
	scoreDivider.w = 10;
	scoreDivider.h = 150;
	SDL_RenderFillRect(renderer, &scoreDivider);

	SDL_Rect rightVerticle;
	rightVerticle.x = windowWidth - 10;
	rightVerticle.y = 0;
	rightVerticle.w = 10;
	rightVerticle.h = windowHeight;
	SDL_RenderFillRect(renderer, &rightVerticle);

	//printing and positioning of scores:
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	ScorePrinting(renderer, f, rightScore, windowWidth - 500, 10); //right
	ScorePrinting(renderer, f, leftScore, 425, 10); //left

	//printing paddles
	SDL_SetRenderDrawColor(renderer, 33, 46, 150, 255); //blue
	SDL_RenderFillRect(renderer, &leftPaddle); //left Paddle

	SDL_SetRenderDrawColor(renderer, 175, 33, 46, 255); //red
	SDL_RenderFillRect(renderer, &rightPaddle); //right Paddle

	//printing ball
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //black ball
	drawingBall(renderer, xBall, yBall, ballRadius, 4);

	SDL_RenderPresent(renderer);
	SDL_RenderClear(renderer);
}


void* ControlPaddle(void* param) //Thread 1
{

	//W for Up Movement
	//S for Down Movement

	bool up = false;
	bool down = false;

	SDL_Event E;

	while (!GameOver)
	{
		while (!SDL_PollEvent(&E));

		if (E.type == SDL_KEYDOWN)
		{
			switch (E.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				GameOver = true;
				break;
			case SDLK_s:
				down = true;
				break;
			case SDLK_w:
				up = true;
				break;
			default:
				break;
			}
		}

		if (E.type == SDL_KEYUP)
		{
			switch (E.key.keysym.sym)
			{
			case SDLK_s:
				down = false;
				break;
			case SDLK_w:
				up = false;
				break;
			default:
				break;
			}
		}

		int taps = 0;

		if (down)
		{
			while (leftPaddle.y + leftPaddle.h < windowHeight - 5 && taps < 18)
			{
				leftPaddle.y = leftPaddle.y + 3;
				taps++;
			}
		}

		taps = 0;

		if (up)
		{
			while (leftPaddle.y > 150 && taps < 18)
			{
				leftPaddle.y = leftPaddle.y - 3;
				taps++;
			}
		}


	}

	pthread_exit(NULL);

}

void* AutomaticPaddle(void* param) //Thread 2: right paddle
{

    int direction = 2; //initially up

    while (!GameOver)
    {
        int taps = 0;

        switch (direction)
        {
            case 1: // down
                while (rightPaddle.y + rightPaddle.h < windowHeight - 5 && taps < 22)
                {
                    rightPaddle.y = rightPaddle.y + 3;
                    taps++;
                }
                break;
            case 2: // up
                while (rightPaddle.y > 150 && taps < 22)
                {
                    rightPaddle.y = rightPaddle.y - 3;
                    taps++;
                }
                break;
        }

       	if (taps < 22)
	{

		if (direction == 2)
			direction = 1;
		else
			direction = 2;

	}

        SDL_Delay(57);
    }

    pthread_exit(NULL);
}

void* Ball(void* param) //Thread 3
{
	int direction = 1; //initially ball moves left

	//direction key:
	//1=left
	//2=right
	//3=downright
	//4=upright
	//5=downleft
	//6=upleft

	while (!GameOver)
	{
		switch (direction)
		{
		case 1: //left
			xBall = xBall - 1;
			break;
		case 2: //right
			xBall = xBall + 1;
			break;
		case 3: //downright
			xBall = xBall + 1; //right
			yBall = yBall + 1; //down
			break;
		case 4: //upright
			xBall = xBall + 1; //right
			yBall = yBall - 1; //up
			break;
		case 5: //downleft
			xBall = xBall - 1; //left
			yBall = yBall + 1; //down
			break;
		case 6: //upleft
			xBall = xBall - 1; //left
			yBall = yBall - 1; //up
			break;
		}

		if ((xBall + ballRadius) >= rightPaddle.x) //ball reaches right side of the screen
		{
			if (((yBall + ballRadius) >= rightPaddle.y) && ((yBall - ballRadius) <= (rightPaddle.y + rightPaddle.h))) //ball within right Paddle range
			{
				if (direction == 2) //ball comes from right
				{
					int high = rightPaddle.y + rightPaddle.h / 3;
					int low = rightPaddle.y + (2 * rightPaddle.h / 3);

					if (yBall <= high)
					{
						direction = 6; //reflect ball upleft
					}
					else if( yBall >= high && yBall <= low) //mid
					{
						direction = 1; //reflect ball right
					}
					else //low
					{
						direction = 5; //reflect ball downleft
					}
					
				}
				else if (direction == 4) // ball comes as upright
				{
					direction = 6; //reflect ball upleft
				}
				else if (direction == 3) //ball comes as downright
				{
					direction = 5; //reflect ball downleft
				}
				
			}
			else if (((rightPaddle.x) <= (xBall + ballRadius - 20))) //ball outside right paddle range
			{
				//ball respawn:
				direction = 1; 
				xBall = windowWidth / 2;
				yBall = (windowHeight + 150) / 2;
				
				//score Increment
				leftScore++;
				
				//Initialisation of right Paddle Coordinates
				rightPaddle.x = windowWidth - rightPaddle.w - 5;
				rightPaddle.y = ((150 + windowHeight) / 2) - (rightPaddle.h / 2);
			
				//Initialisation of right Paddle Coordinates
				leftPaddle.x = 5;
				leftPaddle.y = (150 + windowHeight) / 2 - leftPaddle.h / 2;
			}
		}
		else if (((xBall - ballRadius) <= (leftPaddle.x + leftPaddle.w))) //ball reaches Left Side of the screen
		{
			if (((yBall - ballRadius) <= (leftPaddle.y + leftPaddle.h)) && ((yBall + ballRadius) >= (leftPaddle.y))) //ball within left Paddle range
			{
				if (direction == 1) //ball comes as left
				{
					int high = leftPaddle.y + leftPaddle.h / 3;
					int low = leftPaddle.y + (2 * leftPaddle.h / 3);

					if (yBall <= high)
					{
						direction = 4; //reflect upright
					}
					else if ((yBall >= high) && (yBall <= low)) //mid
					{
						direction = 2; //reflect right
					}
					else //low
					{
						direction = 3; //reflect downright
					}

				}
				else if (direction == 6) //ball comes as upleft
				{
					direction = 4; //reflect ball upright
				}
				else if (direction == 5) //ball comes as downleft
				{
					direction = 3; //reflect ball downright
				}
			}
			else if ((leftPaddle.x + leftPaddle.w) > (xBall - ballRadius + 20)) //ball outside left paddle range
			{
				//respawn ball:
				xBall = windowWidth / 2;
				yBall = (150 + windowHeight) / 2;
				direction = 1;
				
				//Score Increment
				rightScore++;
				
				//Initialisation of right Paddle Coordinates
				rightPaddle.x = windowWidth - rightPaddle.w - 5;
				rightPaddle.y = ((150 + windowHeight) / 2) - (rightPaddle.h / 2);
	
				//Initialisation of right Paddle Coordinates
				leftPaddle.x = 5;
				leftPaddle.y = (150 + windowHeight) / 2 - leftPaddle.h / 2;
			}
		}

		if ((yBall - ballRadius) <= 150) //ball touches upper boundary
		{
			if (direction == 4) //ball comes as upright
				direction = 3; //reflect ball downright
			else if (direction == 6) //ball comes as upleft
				direction = 5; //reflect ball downleft
		}
		else if ((yBall + ballRadius) >= (windowHeight - 5)) //ball touches lower boundary
		{
			if (direction == 3) //ball comes as downright
				direction = 4; //reflect ball upright
			else if (direction == 5) //ball comes as downleft
				direction = 6; //reflect ball upleft
		}

		SDL_Delay(1);
	}

	pthread_exit(NULL);
}

void EndText(SDL_Renderer* renderer, TTF_Font* font)
{
	SDL_Color color;
	SDL_Surface* s;

	if (rightScore == 10 || GameOver == true)
	{
		color = { 255, 0, 0, 255 }; // red
		s = TTF_RenderText_Solid(font, "GAME OVER", color); //losing or escaping text
	}
	else
	{
		color = { 0, 255, 0, 255 }; // green
		s = TTF_RenderText_Solid(font, " YOU WON ", color); //winning text
	}

	SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
	int textWidth = s->w;
	int textHeight = s->h;
	SDL_Rect textBox = { ((windowWidth - textWidth) / 2) - 525, ((windowHeight - textHeight) / 2) - 350, 1200, 700 }; //printing in the centre

	SDL_RenderCopy(renderer, t, NULL, &textBox);
	SDL_FreeSurface(s);
	SDL_DestroyTexture(t);
}

bool EndGame(SDL_Renderer* renderer, TTF_Font* font)
{
	SDL_Event E;
	bool startGame = false;

	int count = 0;
	while (count < 10)
	{

		SDL_SetRenderDrawColor(renderer, 5, 0, 0, 255);
		SDL_RenderClear(renderer);
		EndText(renderer, font);
		SDL_RenderPresent(renderer);
		SDL_Delay(100);
		SDL_RenderClear(renderer);
		count++;
	}
	return true;
}

int main()
{


	//clear screen:

	int id;
	id = fork();

	if (id < 0)
	{
		cout << "Could Not Fork";
	}
	else if (id == 0) //child
	{
		execlp("clear", "clear", NULL);
	}
	else //parent
	{
		wait(NULL);

	}


	//Display Mode:
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	SDL_DisplayMode mode;
	SDL_GetCurrentDisplayMode(0, &mode);
	windowHeight = mode.h;
	windowWidth = mode.w;
	SDL_Window* w = SDL_CreateWindow("22L-6637 Game Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_Renderer* renderer = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
	
	//Initialisation of Ball Coordinates
	xBall = windowWidth / 2;
	yBall = (150 + windowHeight) / 2;
	
	//Initialisation of right Paddle Coordinates
	rightPaddle.x = windowWidth - rightPaddle.w - 5;
	rightPaddle.y = ((150 + windowHeight) / 2) - (rightPaddle.h / 2);
	
	//Initialisation of left Paddle Coordinates
	leftPaddle.x = 5;
	leftPaddle.y = (150 + windowHeight) / 2 - leftPaddle.h / 2;
	
	
	//loading fonts:
	TTF_Font* f = TTF_OpenFont("/usr/share/fonts/truetype/fonts-beng-extra/Ani.ttf", 130); //font for scores
	TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/noto/NotoSansMono-Bold.ttf", 36); //for for main and end screen

	//printing start screen
	bool gameStarted = StartGame(renderer, font);
	if (!gameStarted)
		return 1;

	//Left Paddle Thread
	pthread_t LeftBatTID;
	pthread_create(&LeftBatTID, NULL, &ControlPaddle, NULL);

	//Right Paddle Thread
	pthread_t RightBatTID;
	pthread_create(&RightBatTID, NULL, &AutomaticPaddle, NULL);

	//Ball Thread
	pthread_t BallTID;
	pthread_create(&BallTID, NULL, &Ball, NULL);

	//Layout:
	while (rightScore < 10 && leftScore < 10 && !GameOver)
	{
		Layout(renderer, f);
	}

	//ending screen
	EndGame(renderer, font);

	//closing fonts
	TTF_CloseFont(f);
	TTF_CloseFont(font);
	
	//closing video mode
	SDL_DestroyWindow(w);
	SDL_Quit();

	//Joining Threads
	pthread_join(LeftBatTID, NULL);
	pthread_join(RightBatTID, NULL);
	pthread_join(BallTID, NULL);


	return 0;
}

