#include <stdio.h>
#include <stdlib.h>

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

struct game {
    SDL_Window *pWindow;
    SDL_Renderer * pRenderer;
    TTF_Font *pFont;
    Text *pStartText, *pWaitingText;
    GameState state;
    int playerNumber;
    Player Players[MAX_CLIENTS];

    UDPsocket Socket;
    UDPpacket *pPacket;
    IPaddress serverAddress;
};
typedef struct game Game;

struct clientData{
    Player playerData;
    int playerNumber;
};
typedef struct clientData ClientData;

struct serverData{
    int playerNr;
    GameState gState;
    Player playersData[MAX_CLIENTS];
};
typedef struct serverData ServerData;

void quit(Game *pGame);
void updateWithServerData(Game *pGame);
void createPlayers(Game *pGame);
void drawPlayers(SDL_Renderer *renderer, Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);

int main (int argv, char** args) {
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

    game.pWindow = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
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

    if (!(game.Socket = SDLNet_UDP_Open(0))) {
        printf("SDLNet_UDP_Open: €s\n", SDLNet_GetError());
        return 1;
    }
    if (SDLNet_ResolveHost(&(game.serverAddress), "127.0.0.1", 2000)) { //1
        printf("SDLNet_ResolveHoast(127.0.0.1): %s\n", SDLNet_GetError());
        return 1;
    }
    if (!(game.pPacket = SDLNet_AllocPacket(512))) {
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 1;
    }
    game.pPacket->address.host = game.serverAddress.host;
    game.pPacket->address.port = game.serverAddress.port;

    game.pStartText = createText(game.pRenderer, 238, 168, 65, game.pFont, "Press space to join", WINDOW_WIDTH/2, WINDOW_HEIGHT/2+100);
    game.pWaitingText = createText(game.pRenderer, 238, 168, 65, game.pFont, "Wating for server...", WINDOW_WIDTH/2, WINDOW_HEIGHT/2+100);
    if (!game.pStartText || !game.pWaitingText) {
        printf("Error: %s\n", SDL_GetError());
        quit(&game);
        return 1;
    }

    //createPlayers(&game);

    game.state = START;

    //Start game loop
    int close_requested = 0;
    SDL_Event event;
    ClientData cData;

    int joining = 0;
    while (!close_requested) {
        switch (game.state) {
            case ONGOING:
                SDL_SetRenderDrawColor(game.pRenderer,0,0,0,255);
                SDL_RenderClear(game.pRenderer);
                printf("utanfor\n");
                while(SDLNet_UDP_Recv(game.Socket, game.pPacket)) { //10 //Does not recive anything
                    printf("hdjdk\n");
                    updateWithServerData(&game);
                    SDL_SetRenderDrawColor(game.pRenderer, 0, 255, 0, 255);
                    SDL_RenderFillRect(game.pRenderer, &(SDL_Rect){100, 100, 50, 50});
                }
                if (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) close_requested = 1;
                    else handleInput(&game, &event); //6
                }
                
                drawPlayers(game.pRenderer, &game);
                SDL_RenderPresent(game.pRenderer);
                break;
            
            case START:
                if (!joining) {
                    drawText(game.pStartText);
                } else {
                    SDL_SetRenderDrawColor(game.pRenderer, 0, 0, 0, 255);
                    SDL_RenderClear(game.pRenderer);
                    SDL_SetRenderDrawColor(game.pRenderer, 230, 230, 230, 255);
                    drawText(game.pWaitingText);
                }
                SDL_RenderPresent(game.pRenderer);

                if (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) close_requested = 1;
                    else if (!joining && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                        joining = 1;
                        cData.playerNumber = -1;
                        memcpy(game.pPacket->data, &cData, sizeof(ClientData));
                        game.pPacket->len = sizeof(ClientData);
                    }
                }
                if (joining) SDLNet_UDP_Send(game.Socket, -1, game.pPacket);//2
                if (SDLNet_UDP_Recv(game.Socket, game.pPacket)) {//5
                    updateWithServerData(&game);
                    if (game.state == ONGOING) joining = 0;
                }
        }
    }

}

void updateWithServerData(Game *pGame) {
    ServerData sData;
    memcpy(&sData, pGame->pPacket->data, sizeof(ServerData));
    pGame->playerNumber = sData.playerNr;
    pGame->state = sData.gState;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pGame->Players[i].body = sData.playersData[i].body;
        pGame->Players[i].color = sData.playersData[i].color;
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

void handleInput(Game *pGame, SDL_Event *pEvent) {
    if (pEvent->type == SDL_KEYDOWN) {
        ClientData cData;
        cData.playerNumber = pGame->playerNumber;
        switch (pEvent->key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                pGame->Players[pGame->playerNumber].body.y--;
                break;
        }
        cData.playerData.body = pGame->Players[pGame->playerNumber].body;
        cData.playerData.color = pGame->Players[pGame->playerNumber].color;
        memcpy(pGame->pPacket->data, &cData, sizeof(ClientData));
        pGame->pPacket->len = sizeof(ClientData);
        SDLNet_UDP_Send(pGame->Socket, -1, pGame->pPacket); //7
    }
}

void quit(Game *pGame) {
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pFont) TTF_CloseFont(pGame->pFont);

    if (pGame->pStartText) destroyText(pGame->pStartText);
    if (pGame->pWaitingText) destroyText(pGame->pWaitingText);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}