#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>
#include <text.h>

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 560
#define MAX_CLIENTS 2

struct player {
    SDL_Rect body;
    SDL_Color color;
}; 
typedef struct player Player;

enum gameState {START, ONGOING};
typedef enum gameState GameState;

struct serverData{
    int playerNr;
    GameState gState;
    Player playersData[MAX_CLIENTS];
};
typedef struct serverData ServerData;

struct clientData{
    Player playerData;
    int playerNumber;
};
typedef struct clientData ClientData;

struct game {
    SDL_Window *pWindow;
    SDL_Renderer * pRenderer;
    TTF_Font *pFont;
    Text *pStartText;
    GameState state;
    UDPsocket Socket;
    UDPpacket *pPacket;
    IPaddress clients[MAX_CLIENTS];
    int nrOfClients;
    Player Players[MAX_CLIENTS];

    ServerData sData;
};
typedef struct game Game;

void quit(Game *pGame);
void add(IPaddress address, IPaddress clients[], int *pNrOfClients);
void sendGameData(Game *pGame);
void createPlayers(Game *pGame);
void drawPlayers(SDL_Renderer *renderer, Game *pGame);
void updateServerData(Game *pGame);


int main(int argv, char** args) {
    Game game = {0};
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){
        printf("Error: %s\n",SDL_GetError());
        return 1;
    }
    if(TTF_Init()!=0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 1;
    }
    if (SDLNet_Init())
	{
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
        TTF_Quit();
        SDL_Quit();
		return 1;
	}

    game.pWindow = SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!game.pWindow) {
        printf("Error: %s\n", SDL_GetError());
        quit(&game);
        return 1;
    }
    game.pRenderer = SDL_CreateRenderer(game.pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!game.pRenderer) {
        printf("Error: %s\n", SDL_GetError());
        quit(&game);
        return 1;
    }

    game.pFont = TTF_OpenFont("../lib/resources/arial.ttf", 100);
    if (!game.pFont) {
        printf("Errir: %s\n", TTF_GetError());
        quit(&game);
        return 1;
    }

    if (!(game.Socket = SDLNet_UDP_Open(2000))) {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        quit(&game);
        return 1;
    }

    game.pPacket = SDLNet_AllocPacket(512);
    if (!(game.pPacket)){
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        quit(&game);
        return 1;
    }

    game.pStartText = createText(game.pRenderer, 238, 168, 65, game.pFont, "Waiting for clients", WINDOW_WIDTH/2, WINDOW_HEIGHT/2+100);
    if (!game.pStartText) {
        printf("Error: %s\n", SDL_GetError());
        quit(&game);
        return 1;
    }

    createPlayers(&game);

    game.state = START;
    game.nrOfClients = 0;


    int close_requested = 0;
    SDL_Event event;
    //start of game loop
    while (!close_requested) {
        switch (game.state) {
            case ONGOING:
                SDL_SetRenderDrawColor(game.pRenderer,0,0,0,255);
                SDL_RenderClear(game.pRenderer);
                sendGameData(&game);//4 //9
                while (SDLNet_UDP_Recv(game.Socket, game.pPacket)==1) {
                    updateServerData(&game); //8 
                }
                if(SDL_PollEvent(&event)) if(event.type==SDL_QUIT) close_requested = 1;
                
                drawPlayers(game.pRenderer, &game);
                SDL_RenderPresent(game.pRenderer);
                
                break;
            
            case START:
                drawText(game.pStartText);
                SDL_RenderPresent(game.pRenderer);
                if(SDL_PollEvent(&event)) if(event.type==SDL_QUIT) close_requested = 1;
                if (SDLNet_UDP_Recv(game.Socket, game.pPacket) == 1) {//3
                    add(game.pPacket->address, game.clients, &(game.nrOfClients));
                    if (game.nrOfClients == MAX_CLIENTS) game.state = ONGOING;
                }
                break;
        }
    }
    
}

void add(IPaddress address, IPaddress clients[], int *pNrOfClients) {
    for(int i=0;i<*pNrOfClients;i++) if(address.host==clients[i].host && address.port==clients[i].port) return;
	clients[*pNrOfClients] = address;
	(*pNrOfClients)++;
}

void sendGameData(Game *pGame) {
    pGame->sData.gState = pGame->state;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pGame->sData.playersData[i].body = pGame->Players[i].body;
        pGame->sData.playersData[i].color = pGame->Players[i].color;
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        //printf("client %d\n",i);
        pGame->sData.playerNr = i;
        memcpy(pGame->pPacket->data, &(pGame->sData), sizeof(ServerData));
        pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[i];

        //printf("%d\n", pGame->clients[i].host);
        //printf("%d\n", pGame->clients[i].port);

        SDLNet_UDP_Send(pGame->Socket, -1, pGame->pPacket);
        SDL_SetRenderDrawColor(pGame->pRenderer, 0, 255, 0, 255);
        SDL_RenderFillRect(pGame->pRenderer, &(SDL_Rect){100, 100, 50, 50});

    }
}

void createPlayers(Game *pGame) {
    pGame->Players[0].body = (SDL_Rect){300, 300, 35, 35};
    pGame->Players[0].color = (SDL_Color){255, 0, 0, 255};
    pGame->Players[1].body = (SDL_Rect){340, 300, 35, 35};
    pGame->Players[1].color = (SDL_Color){0, 255, 0, 255};
}

void drawPlayers(SDL_Renderer *renderer, Game *pGame) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        SDL_SetRenderDrawColor(renderer, pGame->Players[i].color.r, pGame->Players[i].color.g, pGame->Players[i].color.b, pGame->Players[i].color.a);
        SDL_RenderFillRect(renderer, &pGame->Players[i].body);
    }
}

void updateServerData(Game *pGame) {
    ClientData cData;
    memcpy(&cData, pGame->pPacket->data, sizeof(ClientData));

    /*printf("%d\n", cData.playerData.body.h);
    printf("%d\n", cData.playerData.body.w);
    printf("%d\n", cData.playerData.body.x);
    printf("%d\n\n", cData.playerData.body.y);*/

    //printf("%d\n", cData.playerNumber);
    if (cData.playerNumber != -1) {
        pGame->Players[cData.playerNumber].body = cData.playerData.body;
        pGame->Players[cData.playerNumber].color = cData.playerData.color;
    }
    
}

void quit(Game *pGame) {
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pFont) TTF_CloseFont(pGame->pFont);
    if (pGame->Socket) SDLNet_UDP_Close(pGame->Socket);
    if (pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);

    if (pGame->pStartText) destroyText(pGame->pStartText);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}