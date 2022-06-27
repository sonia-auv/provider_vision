# Provider_Vision

![Docker Image CI - Master Branch](https://github.com/sonia-auv/provider_vision/workflows/Docker%20Image%20CI%20-%20Master%20Branch/badge.svg)
![Docker Image CI - Develop Branch](https://github.com/sonia-auv/provider_vision/workflows/Docker%20Image%20CI%20-%20Develop%20Branch/badge.svg?branch=develop)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/sonia-auv/provider_vision)
![Average time to resolve an issue](https://isitmaintained.com/badge/resolution/sonia-auv/provider_vision.svg)



One Paragraph of project description goes here

## Getting Started

Clone current project by using following command :
```bash
    git clone git@github.com:sonia-auv/provider_vision.git
```

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

**IMPORTANT :** *If you have just imported your repository, please follow the instructions in* [BOOTSTRAP.md](BOOTSTRAP.md) (Once the bootstrap completed, you can remove this comment from the README)

### Prerequisites

First and foremost to run the module you will need to have [docker](https://www.docker.com/get-started?utm_source=google&utm_medium=cpc&utm_campaign=getstarted&utm_content=sitelink&utm_term=getstarted&utm_budget=growth&gclid=CjwKCAjw57b3BRBlEiwA1Imytuv9VRFX5Z0INBaD3JJNSUmadgQh7ZYWTw_r-yFn2S4XjZTsLbNnnBoCPsIQAvD_BwE) installed.

To validate your installation of docker, simply type in

```
docker -v
```

If you receive an output in the likes of :
```
Docker version 19.03.5, build 633a0ea
```

It means you have it installed. If not follow instructions on how to install it for your OS.

To add a second camera, check the code highlighted in this issue :

https://github.com/neufieldrobotics/spinnaker_sdk_camera_driver/issues/44

### Installing and testing

To test the camera, you can use the repository that has been use to create provider_vision. In the README, they explain how you can test the camera on Ubuntu.

* [Spinnaker SDK Camera Driver](https://github.com/neufieldrobotics/spinnaker_sdk_camera_driver/tree/master)

## Documentation

To find more information on the Spinnaker SDK : [Getting started Spinnaker SDK](https://flir.custhelp.com/app/answers/detail/a_id/4327/~/getting-started-with-the-spinnaker-sdk/session/L2F2LzEvdGltZS8xNjE5NDg5NjUwL2dlbi8xNjE5NDg5NjUwL3NpZC9mVTNiYlNTNDlHNWZNRU5PSjhhQkxYQ21TQUhmRmZNcGdjTXlSaDRvZl9qUzl2M25SWkVDTlNiVTAzTzVieU5qayU3RXllZWNXNFdJUldCNHlGY1lzWHk3cGRER242M1lNaGF4NFJTc2ZRNXoxcTV5b21ONVZsNVo2USUyMSUyMQ==)

## Deployment

To deploy the provider_vision, you will need to get the ID of the camera and put the ID in the config files. You can use **FlirSpinview** to get this information.

## Built With

Add additional project dependencies

* [ROS](http://wiki.ros.org/) - ROS robotic framework


## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags).

## License

This project is licensed under the GNU License - see the [LICENSE](LICENSE) file for details
