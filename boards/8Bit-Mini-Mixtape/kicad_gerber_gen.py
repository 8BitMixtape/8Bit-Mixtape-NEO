'''
    A python script example to create plot files to build a board:
    Gerber files
    Drill files

    source https://github.com/KiCad/kicad-source-mirror/blob/master/demos/python_scripts_examples/plot_board.py
    additons via https://github.com/beagleboard/pocketbeagle/blob/master/kicad-scripts/kicad-fab.py
'''

import sys, os, subprocess

from pcbnew import *
filename=sys.argv[1]
gerbv_path=sys.argv[2]

    
board = LoadBoard(filename)

plotDir = os.path.dirname(filename)+"/plot/"

pctl = PLOT_CONTROLLER(board)

popt = pctl.GetPlotOptions()

popt.SetOutputDirectory(plotDir)

# Set some important plot options:
popt.SetPlotFrameRef(False)     #do not change it
popt.SetLineWidth(FromMM(0.15)) # default line width to plot items having no line thickiness defined

popt.SetAutoScale(False)        #do not change it
popt.SetScale(1)                #do not change it
popt.SetMirror(False)
popt.SetUseGerberAttributes(False) #true will set it to gerber x2, manyboard fab houses dont like this
popt.SetUseGerberProtelExtensions(True) # change extension from default .gbr
popt.SetExcludeEdgeLayer(True); # true will include edge in copper, not good
popt.SetScale(1)
popt.SetUseAuxOrigin(True)

# This by gerbers only
popt.SetSubtractMaskFromSilk(False)
# Disable plot pad holes
popt.SetDrillMarksType( PCB_PLOT_PARAMS.NO_DRILL_SHAPE );
# Skip plot pad NPTH when possible: when drill size and shape == pad size and shape
# usually sel to True for copper layers
popt.SetSkipPlotNPTH_Pads( False );

# param 0 is the layer ID
# param 1 is a string added to the file base name to identify the drawing
# param 2 is a comment
# Create filenames in a way that if they are sorted alphabetically, they
# are shown in exactly the layering the board would look like. So
#   gerbv *
# just makes sense. The drill-file will be numbered 00 so that it is first.
plot_plan = [
    ( Edge_Cuts, "01-Edge_Cuts",   "Edges" ),

    ( F_SilkS,   "02-SilkTop",     "Silk top" ),
    ( F_Mask,    "03-MaskTop",     "Mask top" ),
    ( F_Cu,      "04-CuTop",       "Top layer" ),

    ( B_SilkS,   "05-SilkBottom",  "Silk top" ),
    ( B_Mask,    "06-MaskBottom",  "Mask bottom" ),
    ( B_Cu,      "07-CuBottom",    "Bottom layer" ),
]



for layer_info in plot_plan:
    if layer_info[1] <= B_Cu:
        popt.SetSkipPlotNPTH_Pads( True )
    else:
        popt.SetSkipPlotNPTH_Pads( False )

    pctl.SetLayer(layer_info[0])
    pctl.OpenPlotfile(layer_info[1], PLOT_FORMAT_GERBER, layer_info[2])
    print 'plot %s' % pctl.GetPlotFileName()
    if pctl.PlotLayer() == False:
        print "plot error"

# At the end you have to close the last plot, otherwise you don't know when
# the object will be recycled!
pctl.ClosePlot()

# Fabricators need drill files.
# sometimes a drill map file is asked (for verification purpose)
drlwriter = EXCELLON_WRITER( board )
drlwriter.SetMapFileFormat( PLOT_FORMAT_PDF )

mirror = False
minimalHeader = False
offset = wxPoint(0,0)
# False to generate 2 separate drill files (one for plated holes, one for non plated holes)
# True to generate only one drill file
mergeNPTH = True
drlwriter.SetOptions( mirror, minimalHeader, offset, mergeNPTH )

metricFmt = True
drlwriter.SetFormat( metricFmt )

genDrl = True
genMap = True
print 'create drill and map files in %s' % pctl.GetPlotDirName()
drlwriter.CreateDrillandMapFilesSet( pctl.GetPlotDirName(), genDrl, genMap );


# We can't give just the filename for the name of the drill file at generation
# time, but we do want its name to be a bit different to show up on top.
# So this is an ugly hack to rename the drl-file to have a 0 in the beginning.
base_name = os.path.splitext(os.path.basename(filename))[0]
print base_name

drill_file = os.path.dirname(filename) + "/plot/" + base_name
print 'drill file: '+ drill_file

#if os.path.exists(drill_file+ "-00.drl"):
#    print 'file exists, skip rename'
#else:
#    os.rename(drill_file+ ".drl", drill_file+ "-00.drl")

if os.name == 'nt':
    os.system(gerbv_path+'/gerbv.exe '+ os.path.dirname(filename)+'/plot/*')
    pass # Windows
else:
    os.system('gerbv plot/*')
    pass # other (unix)
