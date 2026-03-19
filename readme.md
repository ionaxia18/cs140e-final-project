# Picraft
**Clara Wang, Erika Li, Iona Xia**  

[Video Demo](https://drive.google.com/file/d/1fVVa6dfv8KzZIKBdKGQ1rL_hryTVF_Vg/view?usp=drive_link)

## Overview:
Our project is Picraft, a system where a Raspberry Pi runs a game server that controls a Minecraft world. The Pi uses physical hardware for input, stores world state on our SD card, and talks to a Minecraft plugin over UART and TCP to display the changes to the server visually.   

<img width="1353" height="564" alt="Data flow in Minecraft bridge system" src="https://github.com/user-attachments/assets/38dca9d1-e98f-4fa1-9fbd-b6f0a335e791" />

## Programs:
Two main bridge programs facilitate the communication between the Pi and Minecraft’s FruitJuice plugin, with the Pi running one program and the laptop running the other.

The Pi program starts by initializing the world (via information from its SD card), GPIO, UART and heap. Through a continuous while loop, it is able to poll information from the joystick and the block matrix. As it gets information, it edits the world state and sends state updates over UART to the laptop with commands for the following:

* BLOCK x y z block\_id (add/remove block)  
* PLAYER x y z (player position)  
* ROT yaw pitch (player rotation)  
* DONE (session end)

The laptop bridge program ensures that changes it is getting from UART gets sent over to the Minecraft FruitJuice Plugin. It starts by connecting to the FruitJuice plugin via TCP and then opens the Pi serial port. It then continuously parses the UART lines and translates them to the plugin.

## World State: 
Our world state is stored in a heap allocated hash table with linked list chaining. Each block has an x, y, z coordinate, and when the player places or deletes a block, we save the change in the table. We use a Knuth multiplicative hash function to hash a concatenated version of the x, y, z coordinates into indices. Then, either add or modify the linked list entry corresponding to that coordinate. We have a destroy world function that frees up the hash table and restores the server to its original form. 

Just like Minecraft, we support world generation via seed. For example, in our server, 0 corresponds to a super flat world. To make our storage efficient, we only store changes to the base world in the hash table since we can query what a coordinate’s base block is via the seed generation function. 

## Game Logic:

The core functionality of Minecraft / Picraft is to allow a player to move around in the world, place blocks, and break blocks. 

To accomplish this, we store information about the player at any given moment, including the position and the camera’s current yaw and pitch. This helps us calculate what direction to move the player in based on their current view direction and hardware input. 

To detect which specific block the player wants to modify, we had to implement raycasting, or tracing a line from a point (the player’s camera) in a specific direction to detect block collisions. We detect if there’s a block at the location the player is looking at to allow players to build on top of blocks. 

## Hardware:

We use two analog joysticks for movement and camera rotation. Since the Pi doesn’t support analog input, we use a MCP3008 as an analog-to-digital converter with an SPI interface. We then read the SPI channels to get the movement and camera displacements. 

The button in the center of each joystick serves as the place / break block input. The button for the movement joystick places blocks, and the other breaks. We used a 10k pull-up resistor to read the inputs accurately.

Finally, we use a button matrix to emulate a player’s inventory. Each button maps to a different type of block that the player can select before pressing the place block button. The last button on the matrix has our quit functionality, prompting the world state to save onto our boot file before ending the current game. 

## Filesystem:

In order to connect our world state to our Pi’s SD card, we extended our FAT32 lab to also have writing capabilities such that we can write a server.bin file to the SD card with the flattened version of the world and player state. When starting the server, the pi will retrieve the existing world state if it exists or create a new world state if the file does not exist. When the player quits the program, the changes get saved by flushing the world state hash table to the boot file. 
