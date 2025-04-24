# För att köra programmet  
Kör make filerna i server och client mapparna.  
Kör exe filerna som skapats (en server, två clients).  
Tryck mellanslag på båda clienterna och vänta på att spelet ska starta.  
Tryck pil upp eller w i clienterna för att flytta på fyrkanten.  

# Problem
Clienten kan skicka data till servern.  
Servern kan ta emot datan.  
Servern kan skicka data till clienten.  
Clienten tar bara emot data under start fasen (line 168) men inte under ongoing fasen (line 133).  
