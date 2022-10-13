# build image with 4 CPUs
docker build --cpuset-cpus="0-3"  -t [USERNAME]/shamon:1.0 .
                                     # above of this text is the image name
# run image
docker run -t -d  --platform=linux/amd64 [image]

# start bash in container to write commands
docker exec -it [CONTAINER ID]  /bin/sh