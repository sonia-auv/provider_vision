# Templated solution
*Remember this is a template repository, you will have to remove this documentation once you are done preparing your repository*

**ROS Package Repo Template** is used to predefine certain values in your repository by importing this repository to your new module.
The template should be used to create a ROS Module. Depending on whether the module you want to create should be used for "perception" modules please follow the right instructions

## Bootstrap your project
Once you have used the `ros-package-repo-template` to create your own package repository you can now use the `get_started.sh` script.

For further informations on how to use the script you can simply run :
```
$ ./get_started.sh -h
```
Which should output :
```
USAGE: ./get_started.sh [flags]
flags:
  -n  The name of your package (Required) (default: '')
  -t  The type of project to be created (e.g. perception) (default: 'perception')
  -g  If the package needs GPU (default: false)
  -c  Command to be executed by the script (e.g. create or prepare) (default: 'create')
  -d  Give extra dependencies separated by commas (e.g. sonia_common,turtlebot3,qt_dotgraph) (default: 'sonia_common')
  -h  show this help (default: false)
```
### Preparing your repository
**IMPORTANT** *: We recommend using the `ros:melodic-ros-core` in order to run the script. This allows to have a valid and genuine installation of ROS to execute the script.*
*If you already have ROS installed on your computer you can try to simply run the script `./get_started.sh [flags]`*

#### Project perception (non-GPU)
```
$ cd /path/to/your/repository
$ docker run --rm -it -v $(pwd):/tmp  ros:melodic-ros-core /bin/sh -c 'cd /tmp && ./get_started.sh -n [your_desired_node_name] -c prepare'
```
#### Project perception (GPU required)
```
$ cd path/to/your/repository
$ docker run --rm -it -v $(pwd):/tmp  ros:melodic-ros-core /bin/sh -c 'cd /tmp && ./get_started.sh -n [your_desired_node_name] -c prepare -g true'
```

### Creating your ROS basic files
#### Standard procedure
By default it is recommend to only have `sonia_common` as a dependency. To create your ROS files for a standard module run :
```
$ cd /path/to/your/repository
$ docker run --rm -it -v $(pwd):/tmp  ros:melodic-ros-core /bin/sh -c 'cd /tmp && ./get_started.sh -n [your_desired_node_name] -c create'
```
#### Specifying extra dependencies
In some cases you might need extra dependencies. The flag `-d` allows to override the included dependencies. Here is an example on how to specify your own dependencies (no spaces and seperated by commas).
```
$ cd /path/to/your/repository
$ docker run --rm -it -v $(pwd):/tmp  ros:melodic-ros-core /bin/sh -c 'cd /tmp && ./get_started.sh -n [your_desired_node_name] -c create -d sonia_common,turtlebot3,qt_dotgraph'
```
*NOTE : In most cases you will need to include `sonia_common`. In doubt, leave it in the list of dependencies.*
