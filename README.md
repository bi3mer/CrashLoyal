Crash Loyal is a clone of a famous moble app game. This project is not to be
used as a comercial product, simply as a teaching tool for 4150 Game AI.

To build and run the project, open a terminal into this root directory and run
the following command:

g++.exe *.cpp -I./include/SDL2 -L./lib -w -lmingw32 -lSDL2main -lSDL2
-lSDL2_image -o crashloyal

This will generate an executable called 'crashloyal' (windows may make it an
exe, linux or mac may not). The executable will initialize the game state
world, a screen as well as begin scanning for use input.

For more details on the graphics/ application library used please check out
the SDL documentation: https://wiki.libsdl.org/FrontPage

For some concrete examples on how to use SDL, please check out Lazy Foo:
http://lazyfoo.net/tutorials/SDL/index.php


##  HW Notes

I ended up changing the structure of the code a decent bit. I didn't like that collisions weren't handled during attacking. So I put the collision handling code in the start of the `Update`. This did mean, though, that collisions could get in the way of attacking. It ended up really slowing the game down so I updated the `moveTowards` function to weight all the different movements. The result is that the mob will always move towards their target but the collisions can have higher or less weights. As it stand the buildings and the river can never be overriden. However, the mobs may overlap eachother a tad bit while going to a target simply to keep the game going forward. If I wanted the collisions to be perfect, then I could up the weights (really lower since I divide) and then the collisions would always take precedence. However, I prefer this approach since it serves the gameplay.

Additionally, I ended up changing the way you gave us functions to handle this. I ended up making one function called `handleCollisions` which populated three points I added to the mob class: `riverCollisionPoint`, `mobCollisionPoint`, and `buildingCollisionPoint`. Each of these are points in space for where the mob is going to be repulsed from. For rectangle collision I found the four corners of the mob and tested if any of the points were inside of the rectangle. I used this for bridges. The bridge collision was only tested if the y coordinate of the mob was in the river. And for squares I used a formula that I remembered from who knows when and I don't remember if there was a name. 

River collisions have an additional feature where the point chosen in space is chosen strategically to push the mob towards the bridge that is closest to them. This means that mobs will not be wondering off the screen and never get back. 

Also, I pulled from the repo and couldn't get it working. So I used a bisect and found the most recent commit that worked on my computer. I also changed this to a markdown file so it renders nicely on github.

The one thing I would really like to improve is to add a gurantee that nothing will get stuck. Occasionally on the bridge there can be an edge case where a mob is on the wrong side of the bridge and wants to go back to its most recent waypoint but cannot. THe result is it gets stuck while trying to move into the river to get there. The way that this resolves itself is by adding an enemy on the screen. It also will technically work for when a giant is added to the game but it would require a tad bit more code to get to function. Ideally, I could set it up to the use the vector of objects to use weights of objects to be a bit smarter in its approach. As it stands, I would need to make a modification to the code or add a special case. Given that I'm not planning long term for this, I'm okay that I left it with the need for a special case.