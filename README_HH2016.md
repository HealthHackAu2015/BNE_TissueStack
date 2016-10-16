
#3D annotation of Terrascale Biomedical Imaging Data in a 2D Web

##Health Hack 2016 - Brisbane

##Team Members
* Andrew Janke - Problem Owner - https://www.linkedin.com/in/ajanke
* Ellot Smith - Hacker - https://www.linkedin.com/in/elliot-smith-373a1740
* Matthew Brown - Hacker - https://www.linkedin.com/in/mnbbrown

##The problem
http://www.healthhack.com.au/challenges#c2

##The Pitch
Biomedical Image segmentation is a major roadblock to advances in neuroscience. Data sizes are 
increasing but the tools to analyse them have not kept pace.

For example to trace a single high resolution 1200 slice mouse model it has taken over 8 months 
of intensive work using traditional tools. The tools are limited to 2D, there is no easy way to 
collaborate and data file size is huge. Typically in the GB range but sometimes up to the TB scale. 
Once complete these tracings need to be constantly updated, revised and annotated.

Even viewing the data is a problem, tools are limited and operating system bound. THere is an 
existing open source online tiled viewing platform (www.tissuestack.org) that allows us to share 
and collaboratively view datasets, this has revolutionised the way we work with data as it has 
allowed researchers to view all their data instead of being limited to a few views. But 
researchers are now requesting more. They want to collaborate on annotations and paintings, 
they want to share and they want to be able to do it from any platform.

Online viewing has also allowed Citizen Science by allowing us to disseminate images to both schools 
and individuals (CSIRO scientists in schools). Viewing of data in museums, old fossils, 
tracing/identification of fossils. The possibilities for online painting and collaboration are immense 
but we don't have the tools.

The problem of tracing isn't hard but the tools to do it collaboratively and in 3D don't exist.

The image below demonstrates the existing style of desktop interface on the left that is used for
small files (4GB Max) and the current web based interface that supports view but not editing of
information.

![HH2016 image](https://github.com/HealthHackAu2016/BNE_TissueStack/blob/master/HH-2016-janke-90.png "HH2016 image")

##Design
The POC online collaborative painting tool that has been developed can be used in many other fields that
use blocks of data. Mining, weather, exploration, more broad areas of biomedical imaging (microscopy) and R+D.
By allowing colalborative online editing.

This POC wil allow for citizen scientists to contribute to areas of science that they were previously 
liminted based upon the available tools. For example Zooniverse allows annotation but it is only 2D and not
collaborative.

##Implementation
This weekend we have delivered a POC implementation that allows for collaborative editing. 

The solution tackles the most difficult part of the existing problem owners pitch. The solution is a working
prototype within the presented problems codebase and is presented as a pull reqest on this fork.

##HH2016 and Open Knowledge values
The proposed solution is Open Source (GPL)
...

