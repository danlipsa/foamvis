/**
 * \mainpage The FoamVis Project
 * \section intro_sec Introduction
 *
 * This project visualizes and helps gaining insights into foam
 * simulation data produced by Surface Evolver.
 *
 * Source files and libraries:
 * http://csgalati.swansea.ac.uk/foam/build
 *
 * Mac Os X binary release:
 * http://csgalati.swansea.ac.uk/foam/bin/mac-osx-10.6.8/
 *
 * The folowing section lists the content of README.txt
 * \verbinclude README.txt
 *
 * \section sec_torus Processing done for the Torus model
 * \subsection sec_onedge Executed when creating an edge (may create duplicate vertices)
 *
 <pre>
The begin vertex (and the middle vertex in quadratic mode) of an edge is 
always defined in the data file (it's not a duplicate).
  if (edge is marked with a *)
    the end vertex is defined in the data file (no DUPLICATE needed)
  else if (edge is marked with a + or -) {
    create a DUPLICATE of the end vertex by translating it
      one domain up for a + and one domain down for a - along each of 
      the three axes. The translation is done relative to the domain where the 
      ORIGINAL end vertex is defined in the data file. This means that
      (* * *) is the domain where the begin vertex is defined in the data file
      NOT the original domain.
  }
 </pre>
 * 
 * \subsection sec_onface Executed when creating a face (may create duplicate edges)
 *
<pre>
  first vertex of the face is defined in the data file (not a DUPLICATE).
  set beginVertex to be the first vertex of the face
  foreach (currentEdge, edges in the face) {
    if (the beginVertex does not match the begin vertex of the currentEdge) {
      create a DUPLICATE of currentEdge starting at beginVertex
      set currentEdge to point to the DUPLICATE
    }
    set beginVertex  to be the end vertex of currentEdge
  }
</pre>
 * 
 * \subsection sec_onbody Executed when creating a body (may create duplicate 
   faces)
 *
<pre>
Add all adjacent faces of face 0 to a queue.
while (no more items in the queue)
{
   remove a adjacent face, translate it if needed and mark it visited.
   if several faces fit, choose the face with smallest angle between its 
   normal and the original face normal.
}

</pre>
 *
 * \section sec_physical_tesselation Physical and tesselation edges and vertices
 * In 2D we don't have physical edges. A vertex is "physical" if has
 * >= 3 edges adjacent to it.
 *
 * In 3D, an edge is physical if it has 6 AdjacentOrientedFace is part of. (is
 * adjacent with 3 faces)
 * An vertex is physical if it has 4 physical edges adjacent to it.
 *
 *
 * \section sec_space Significant space
 * For matrices of expressions and for vertex components
 * x, y, z.
 *
 * \section sec_new_line Significant new line
 * For arrays (2d versus 3d)
 *
 * \section sec_t1s Format of additional text file containing T1s
 * A line that starts with a # is a comment line \n
 * Each line contains three entries separated by space: time_step, x, y
 * where x and y are the coordinates of the T1 in object space \n
 * The first time step is 1. \n
 * A T1 labeled with timestep T occurs between T and T+1.\n
 *
 *
 *
 * @todo Look at paths coresponding to bubbles around the circulation
 *       shown by average: Simon's email 21 Sep. 2012
 * @todo Add vorticity: Simon's email 21 Sep. 2012
 * @todo Implement VIEW_TRANSFORM, VIEW_TRANSFORM_GENERATORS, TRANSFORM_EXPR for
 *       twogascoarse dataset.
 * @todo Why time displacement does not go all the way to max Z?
 * @todo Use vertex arrays for center paths
 * @todo Use google-breakpad for reporting crashes.  
 * @todo Set new line significant only for list of vertices, edges,
 *       faces, bodies
 * @todo Replace vector + tensor drawing with geometry shader or CPU 
 *       (instead fragment shader)
 * @todo optimization: for 2D foam, store vector count in the same texture
 *       instead of storing it in the scalar texture.
 * @todo bug: velocity average shows a black blob if only one time step of data 
         is loaded. An error message should be shown instead.
 * @todo bug: fix Overlay, Velocity, Glyph, Grid Cell Center
 * @todo bug: Translate grid does not work with streamlines.
 * @todo fix the (slow) movement of the focus in context view for the GL view
 * @todo replace glScale with camera movement for GL view
 */
