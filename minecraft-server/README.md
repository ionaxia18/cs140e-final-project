Step 1: Install Java
    brew install openjdk@21 
    Check Version
        java -version
    If version is not 21, run these commands in terminal:
        brew link --force --overwrite openjdk@21 
        echo 'export JAVA_HOME=/opt/homebrew/opt/openjdk@21' >> ~/.zshrc
        echo 'export PATH=$JAVA_HOME/bin:$PATH' >> ~/.zshrc
        source ~/.zshrc

Step 2: Install Official Minecraft Server
Download minecraft_server.1.21.11
https://www.minecraft.net/en-us/download/server 

Step 3: Install Paper server:
https://papermc.io/downloads

Step 4: Drag installed .jar files from Step 2 and 3 into minecraft-server folder

Step 5: Download FruitJuice 0.3.0
https://github.com/jdeast/FruitJuice/blob/master/target/

Step 6: Move .jar file into minecraft-server/plugins

Step 7: Run server from minecraft-server folder
java -Xmx2048M -Xms2048M -jar paper-1.21.11-126.jar 
    Might need to edit name of the file so it matches your filename

Step 8: Download and start up Minecraft
Select Multiplayer
Select Add Server, enter this ip address. Will load you into the world. 
This only works if the ngrok tunnel is running.
8.tcp.us-cal-1.ngrok.io:19389

