#!/bin/bash
# ------------------------------------------------------------------
# Author : Kevin Charbonneau
#          *get_started.sh*
#          This script allows the users to create a package from the
#          ROS package repo template.
#
#          This script uses shFlags -- Advanced command-line flag
#          library for Unix shell scripts.
#          http://code.google.com/p/shflags/
#
# Dependency:
#     http://shflags.googlecode.com/svn/trunk/source/1.0/src/shflags
# ------------------------------------------------------------------

. ./scripts/shflags

DEFINE_string 'name' '' 'The name of your package (Required)' 'n'
DEFINE_string 'type' 'perception' 'The type of project to be created (e.g. perception or other)' 't'
DEFINE_boolean 'gpu' 'false' 'If the package needs GPU' 'g'
DEFINE_string 'command' 'create' 'Command to be executed by the script (e.g. create or prepare)' 'c'
DEFINE_string 'dep' 'sonia_common' 'Give extra dependencies separated by colons (e.g. sonia_common,turtlebot3,qt_dotgraph)' 'd'
FLAGS_HELP="USAGE: $0 [flags]"

main() {
    if [ -z $FLAGS_name ]; then
        echo "You must provide a name for your package"
        echo ""
        flags_help
        exit 1
    fi
    if [ $FLAGS_command == "create" ]; then
        CREATE_PKG_COMMAND="catkin_create_pkg ${FLAGS_name}"
        IFS=',' read -ra ADDR <<< "$FLAGS_dep"
        for i in "${ADDR[@]}"; do
            CREATE_PKG_COMMAND="${CREATE_PKG_COMMAND} ${i}"
        done
        $CREATE_PKG_COMMAND
        mv $FLAGS_name/* .
        rm -r $FLAGS_name
    elif [ $FLAGS_command == "prepare" ]; then
        clean_workflows "$@"
        insert_name "$@"
    else
        echo "The command '${FLAGS_command}' does not exist"
        echo ""
        flags_help
        exit 1
    fi
}

clean_workflows() {
    if [ $FLAGS_gpu == true ]; then
        if [ $FLAGS_type == "perception" ]; then
            rm .github/workflows/docker-image-perception-feature.yml
            rm .github/workflows/docker-image-perception-develop.yml
            rm .github/workflows/docker-image-perception-master.yml
        else
            echo "${FLAGS_type} is an invalid type of package."
            echo "Provide either perception or other"
            echo ""
            flags_help
            exit 1
        fi
    else
        if [ $FLAGS_type == "perception" ]; then
            rm .github/workflows/docker-image-perception-l4t*
        else
            echo "${FLAGS_type} is an invalid type of package."
            echo "Provide either perception or other"
            echo ""
            flags_help
            exit 1
        fi
    fi
}

insert_name() {
    for i in .github/workflows/*; do
        sed -i.bak "s/<ENTER_YOUR_MODULE_NAME>/${FLAGS_name}/g" "${i}" && rm "${i}.bak"
    done

    sed -i.bak "s/<ENTER_YOUR_NODE_NAME>/${FLAGS_name}/g" "Dockerfile" && rm Dockerfile.bak
    sed -i.bak "s/<ENTER_YOUR_NODE_NAME>/${FLAGS_name}/g" ".vscode/launch.json" && rm .vscode/launch.json.bak
    sed -i.bak "s/<ENTER_YOUR_NODE_NAME>/${FLAGS_name}/g" ".devcontainer/devcontainer.json" && rm .devcontainer/devcontainer.json.bak
    sed -i.bak "s/<ENTER_YOUR_NODE_NAME>/${FLAGS_name}/g" "README.md" && rm README.md.bak
    sed -i.bak "s/<ENTER_YOUR_NODE_NAME>/${FLAGS_name}/g" "LICENSE" && rm LICENSE.bak
    sed -i.bak "s/<CURRENT_YEAR>/$(date +'%Y')/g" "LICENSE" && rm LICENSE.bak
}

FLAGS "$@" || exit $?
eval set -- "${FLAGS_ARGV}"
main "$@"
# TODO: It could be good to add error handling and prints to the script
