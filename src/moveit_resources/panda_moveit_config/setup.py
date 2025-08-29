from setuptools import setup
from glob import glob
import os

package_name = 'moveit_resources_panda_moveit_config'

def files_under(root_dir):
    out = []
    if os.path.isdir(root_dir):
        for dp, _, fns in os.walk(root_dir):
            for f in fns:
                out.append(os.path.join(dp, f))
    return out

data_files = [
    # ament index: lets ROS find the package
    (os.path.join('share', 'ament_index', 'resource_index', 'packages'),
     [os.path.join('resource', package_name)]),

    # keep package.xml in share
    (os.path.join('share', package_name), ['package.xml']),

    # install launch + rviz
    (os.path.join('share', package_name, 'launch'), glob('launch/*.py')),
    (os.path.join('share', package_name), glob('launch/*.rviz')),  # moveit.rviz is here

    # install config (yaml/xacro/srdf, etc.) recursively
    (os.path.join('share', package_name, 'config'), files_under('config')),
]

setup(
    name=package_name,
    version='3.1.1',
    packages=[package_name],  # empty python pkg is fine
    data_files=data_files,
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Mike Lautman',
    maintainer_email='mike@picknik.ai',
    description='MoveIt config for the Franka Panda (ament_python).',
    license='BSD',
)
