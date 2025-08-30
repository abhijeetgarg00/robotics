from setuptools import find_packages, setup
from glob import glob
import os

package_name = 'arm_full'

def files_under(root):
    out = []
    if os.path.isdir(root):
        for dp, _, fns in os.walk(root):
            for f in fns:
                out.append(os.path.join(dp, f))
    return out


setup(
    name=package_name,
    version='3.1.1',
    packages=find_packages(exclude=['test']),
    data_files=[
        (os.path.join('share', 'ament_index', 'resource_index', 'packages'),
        [os.path.join('resource', package_name)]),
        (os.path.join('share', package_name), ['package.xml']),

        # Install from the source package folder explicitly:
        (os.path.join('share', package_name, 'urdf'),       files_under('arm_full/urdf')),
        (os.path.join('share', package_name, 'meshes/visual'),   files_under('arm_full/meshes/visual')),
        (os.path.join('share', package_name, 'meshes/collision'), files_under('arm_full/meshes/collision')),

        # launch + rviz
        (os.path.join('share', package_name, 'launch'), glob('arm_full/launch/*.py')),
        (os.path.join('share', package_name, 'launch'), glob('arm_full/launch/*.rviz')),

        # configs (yaml/xacro/srdf)
        (os.path.join('share', package_name, 'config'), files_under('arm_full/config')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='beast',
    maintainer_email='abhijeetgarg007@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        ],
    },
)
