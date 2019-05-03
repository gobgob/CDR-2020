# Stratégie chariot-élévateur CDR2019

## Point de départ de match :
|     |Violet|Orange|
|-----|------|------|
|**x**|1210  |-1210 |
|**y**|1400  |1400  |
|**o**|3.14  |0     |

_(Par la suite, on se considère du côté Violet)_
> Attention: la coordonnée `y` de l'actionneur doit être symétrisée pour passer côté orange


## 1. Libération du Goldenium
Point de départ du script : `x=-135; y=1690; o=pi/2`

* Actuator GoTo : `y=-23.7; z=152; theta=0`
* Robot GoTo : `x=-135; y=1740; o=pi/2`
* Actuator GoTo at Speed : `y=23.7; z=190; theta=0; s_y=350; s_z=300; s_theta=1023`
* Robot GoTo : `x=-135; y=1690; o=pi/2`
* Actuator GoHome


## 2. Prise du Goldenium
Point de départ du script : `x=-725; y=1665; o=pi/2`

* Actuator GoTo : `y=-23.7; z=200; theta=0`
* Actuator ScanPuck
* Actuator GoTo : `y=[result from scan]; z=180; theta=0`
* Robot GoTo : `x=-725; y=1740; o=pi/2`
* Actuator GoTo at Speed : `y=[result from scan]; z=180; theta=15; s_y=1023; s_z=300; s_theta=900`
* Robot GoTo : `x=-725; y=1665; o=pi/2`
* Actuator GoTo at Speed : `y=0; z=180; theta=15; s_y=1023; s_z=300; s_theta=900`


## 3. Livraison du Goldenium
