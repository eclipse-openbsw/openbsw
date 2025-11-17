# Docker Information

> [!WARNING]
> The docker setup is currently in re-work.

Developer Dockerfile can be used to set up the development environment.
It includes all the necessary tools, libraries, and dependencies required
for development. This ensures that all developers work in a consistent
environment, reducing the "it works on my machine" problem.
Documentation Dockerfile is used to build project's documentation.

Project uses Docker Compose to manage both the Developer Dockerfile and the
Documentation Dockerfile. Docker Compose simplifies the process of building
and running multiple Docker containers, ensuring that both the development
environment and the documentation container are consistently set up and managed.

## Build the docker image

If you have docker compose installed, you can use this command

    docker compose -f docker/docker-compose.yaml build --build-arg user_id=$(id -u) 

If you do not want to use docker compose, you can use this command instead

    docker build --build-arg user_id=$(id -u) -f docker/<dockerfile_name> -t <image_name> .

This will build the image from the Dockerfile in the current directory (the `.`).

## Run the docker container

If you have docker compose installed, the easiest way is to call

    docker compose -f docker/docker-compose.yaml run --user $(id -u):4996 <service_name>

If you do not want to use docker compose, you can use this command instead

    docker run -it --rm -v "$PWD:/home/jenkins" -w /home/jenkins --user $(id -u):4996 <image_name>

This command creates the docker container and enters it (`-it` for interactive).
The container will be removed when you leave it again (`--rm`).
Your current directory is mounted into the container.
