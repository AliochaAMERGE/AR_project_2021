int id_chord[N] = {2, 7, 13, 14, 21, 38, 42, 48, 51, 59};
  int fingers[N][M][2] = {
      {{7, 2}, {7, 2}, {7, 2}, {13, 3}, {21, 5}, {38, 6}},      // 2
      {{13, 3}, {13, 3}, {13, 3}, {21, 5}, {38, 6}, {42, 7}},   // 7
      {{14, 4}, {21, 5}, {21, 5}, {38, 6}, {38, 6}, {48, 8}},   // 13
      {{21, 5}, {21, 5}, {21, 5}, {38, 6}, {38, 6}, {48, 8}},   // 14
      {{38, 6}, {38, 6}, {38, 6}, {38, 6}, {38, 6}, {59, 10}},  // 21
      {{42, 7}, {42, 7}, {42, 7}, {48, 8}, {59, 10}, {7, 2}},   // 38
      {{48, 8}, {48, 8}, {48, 8}, {51, 9}, {59, 10}, {13, 3}},  // 42
      {{51, 9}, {51, 9}, {59, 10}, {59, 10}, {2, 1}, {21, 5}},  // 48
      {{59, 10}, {59, 10}, {59, 10}, {2, 1}, {7, 2}, {21, 5}},  // 51
      {{2, 1}, {2, 1}, {2, 1}, {7, 2}, {13, 3}, {38, 6}}        // 59
  };


# initiateur : 7
# recherche de la clé 0

table des fingers-7 : 
13-21-38-42

- 7 :

receive initiate LOOKUP 
{0, xxx}
7 : initiate_lookup(0)
7 : lookup(7,0)
message(7,0)

next = findnext(0)
next = 42

SEND : {7,0} -> 42 TAG_LOOKUP

- 42 :
table des fingers-42 : 
48-51-59-13
receive TAG LOOKUP
{7,0}
lookup(7,0)
message(7,0)
next = findnext(0)
next = 59

SEND : {7,0} -> 59 TAG_LOOKUP

- 59 :
table des fingers-59 : 
2-7-13-38
receive TAG LOOKUP
{7,0}
lookup(7,0)
message (7,0)
next = findnext(0)
next = NIL
SEND fingers[0][0] : 2 -> {7,0} TAG LAST CHANCE

- 2 :
7-13-21-38
receive TAG LAST CHANCE
{7,0}
have data = true
message = {7,2}

next_pair = findnext(7)
next_pair = NIL

SEND fingers[0][0] : 2 -> {7,2} TAG SUCC





Id_chord 31 ## message{1,25} ## status.Source = 5
Le responsable de la donnée 25 est (<P1>, <Pair_1>)

[0] = -1        [1] = 1         [2] = 7         [3] = 10        [4] = 20        [5] = 25        [6] = 31        [7] = 34        [8] = 42        [9] = 43        [10] = 60       Fin du simulateur
Recherche de : 25
Initiateur : 1
receive = = = =  2