# KINGDOM OF COLORS#

## TOOLS ##

- Open CV
- Open GL
- GL Mathmatics
- FreeType Library for displaying text


## MARKER TRACKING ##

We used a Aruco marker dictionary (DICT_4X4_250) to easily identify markers by using the identify function of said marker dictionary.


## SETUP ##

We are using a Microsoft Kinect One as a camera from above. 
Hexagon Cards with markers on each edge are used to track neighbouring edges.


## Game ##

The player has a stack of hexagonal cards. There are three colors - blue, yellow and red. A card has either only one, two or three different colors. E.g. if a hexagon has three colors, it is divided into three equally sized areas, each including two Markers.

The player puts these hexagon markers next to each other under the Kinect camera. The goal is, to match same colored edges next to each other in order to get ah higher score. On top of that, the players gets missions they can follow to get even more points.
