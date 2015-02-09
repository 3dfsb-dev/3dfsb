Vision for 3DFSB
================
- 3DFSB provides an immersive virtual 3D world where you can browse through, organize and interact with your digital objects.
- This means you can read books, look at pictures, listen to a CD, handle all kinds of things in-world so that you can have experiences with your digital objects instead of just being a tool that you use to start up other programs.
- In the 3D world, you can start a computer and use it to run "classic" programs, if you want.

- Your objects have a persistent visualization state in the 3D world, so you always find them where you left them, and how you left them when you exited 3DFSB. The visualization state of the object *follows* the physical state, not the other way around. Reason: the fact that things have a place, is really handy. Objects should be movable and the location kept in the visualization state.

Plans for *after* we've moved to a powerful, free open source 3D engine:
------------------------------------------------------------------------
- Better directory visualization by showing a rough view of the contents of the folder. The closer you come to a directory, the move you'll be "zooming in" to the directory folder until you are in the new folder, very naturally. Note: when this task is done, we'll be able to start working on the "show zipfiles as directories" task.
- Improve process visualization (/proc) as animated things, that are stuck/frozen when they are stopped, and that you can also kill!
- Move to trash by labeling things (make them fade away) or pulling a trash bag over them and then having a cool animated "trash collector" pick up the file and move it to your trash folder.
- More device file visualizations
- In-world text editor
- Move/copy/cut/paste tool by dragging one or more objects and duplicating them
- Plugin framework for tools:
    - Each tool is a plugin that offers common tool services: thumbnailing (previewing), "opening" / activating and tool visualisations
    - The delete tool that thumbnails items, and can delete them
    - The "2D open" tool that opens the files in a 2D desktop with your default handler, just like your normal programs would do
    - The "3D open" tool that plays videos in the 3D world, shows webcams, shows other visualisations 

Smaller / Nice to have:
-----------------------
- Speed up image loading: http://search.cpan.org/~bfree/OpenGL-Image-1.02/Image.pod ?
- replace strange hit-calculation with ray-box intersection: http://stackoverflow.com/questions/13534519/how-to-calculate-where-bullet-hits


