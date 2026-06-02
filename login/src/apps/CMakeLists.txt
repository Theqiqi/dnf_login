# src/apps/CMakeLists.txt

# Route the build processing straight into the active CLI console application directory
add_subdirectory(cli)

# NOTE: When you begin building your graphical interface down the road, 
# simply place your source files inside src/apps/ui/ and uncomment the line below:
# add_subdirectory(ui)
