# Stratégie chariot-élévateur CDR2019

## Point de départ de match :
|     |Violet|Orange|
|-----|------|------|
|**x**|1210  |-1210 |
|**y**|1400  |1400  |
|**o**|3.14  |0     |

_(Par la suite, on se considère du côté Violet)_
> Attention: la coordonnée `y` de l'actionneur doit être symétrisée pour passer côté orange

## 1. Libération du goldenium
Point de départ du script : `x=-135; y=1790; o=pi/2`

* Actuator GoTo : `y=-23.7; z=152; theta=0`
* Robot GoTo : `x=-135; y=1840; o=pi/2`
* Actuator GoTo at Speed : `y=23.7; z=225; theta=0; s_y=350; s_z=300; s_theta=1023`
* Robot GoTo : `x=-135; y=1790; o=pi/2`
* Actuator GoHome
