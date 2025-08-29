from setuptools import setup
from glob import glob
import os

package_name = 'moveit_resources_panda_description'

def files_under(root):
    out = []
    if os.path.isdir(root):
        for dp, _, fns in os.walk(root):
            for f in fns:
                out.append(os.path.join(dp, f))
    return out

data_files = [
    # ament index entry so ROS can find the package
    (os.path.join('share', 'ament_index', 'resource_index', 'packages'),
     [os.path.join('resource', package_name)]),

    # keep package.xml in share
    (os.path.join('share', package_name), ['package.xml']),

    # install URDF/Xacro and meshes (recursively)
    (os.path.join('share', package_name, 'urdf'), files_under('urdf')),
    (os.path.join('share', package_name, 'meshes'), files_under('meshes')),
]

setup(
    name=package_name,
    version='3.1.1',
    packages=[package_name],  # empty python package
    data_files=data_files,
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Mike Lautman',
    maintainer_email='mike@picknik.ai',
    description='Franka Panda robot description (URDF/Xacro + meshes).',
    license='BSD',
)
