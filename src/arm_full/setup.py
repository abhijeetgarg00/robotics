# setup.py (only the data_files part changes)

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

def subdirs(root):
    return [os.path.join(root, d) for d in os.listdir(root) if os.path.isdir(os.path.join(root, d))]

data_files = [
    (os.path.join('share', 'ament_index', 'resource_index', 'packages'),
     [os.path.join('resource', package_name)]),
    (os.path.join('share', package_name), ['package.xml']),

    # URDF + meshes
    (os.path.join('share', package_name, 'urdf'),              files_under('urdf')),
    (os.path.join('share', package_name, 'meshes/visual'),     files_under('meshes/visual')),
    (os.path.join('share', package_name, 'meshes/collision'),  files_under('meshes/collision')),

    # launch, configs, worlds
    (os.path.join('share', package_name, 'launch'),  files_under('launch')),
    (os.path.join('share', package_name, 'config'),  files_under('config')),
    (os.path.join('share', package_name, 'worlds'),  files_under('worlds')),
]

# ✅ install each model folder as its own directory (preserve structure)
for mdir in subdirs('models'):
    data_files.append(
        (os.path.join('share', package_name, 'models', os.path.basename(mdir)),
         files_under(mdir))
    )

setup(
    name=package_name,
    version='3.1.1',
    packages=find_packages(exclude=['test']),
    data_files=data_files,
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='beast',
    maintainer_email='abhijeetgarg007@gmail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={},
)
