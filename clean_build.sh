#!/bin/bash

rm -rf /root/opt-dev/include/derecho

rm -rf /root/opt-dev/include/cascade

rm -rf /root/opt-dev/lib/libderecho*

rm -rf /root/opt-dev/lib/libcascade*

rm -rf /root/opt-dev/lib/cmake/cascade

rm -rf /root/opt-dev/lib/cmake/derecho

rm -rf /root/opt-dev/bin/cascade*

rm -rf /root/opt-dev/share/derecho



echo "removed derecho, cascade dependencies"



if [ $# -gt 0 ] && [ "$1" = "true" ]; then

	echo "removed cascade_py packages"

	rm -rf /root/.local/lib/python3.10/site-packages/derecho

	rm -rf /root/.local/lib/python3.10/site-packages/derecho.cascade-1.0rc0.dist-info

fi
