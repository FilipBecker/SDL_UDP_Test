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
Har upptäckt att det beror på att när servern hämtar data (line 132) så skriver den över de sparade klienters ipaddresser vilket den gör efter att den skickat data en gång.  
Har upptäckt att problemet beror på att saker händer ur ordning. När clienten skickar paket för att kopplas till servern så slutar den inte skicka sådana paket tills den får ett paket som säger att den ska starta spelet. Vilket gör att servern hinner starta spelet innan clienten gör det och av den anledningen tar upp paket som den tror är spelardata men egentligen är konektion paket. Konektion paket har playerNumber -1 vilket gör att när servern försöket uppdatera spelaren utifrån playerNumber så hamnar den utanför den arrayen och skriver istället över clienternas ipadresser som ligger en int frammför spelar arrayen i minnet.   
