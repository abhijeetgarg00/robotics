
import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch.conditions import IfCondition
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder

from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.actions import SetEnvironmentVariable
from launch.substitutions import EnvironmentVariable 


def generate_launch_description():


    models_dir = PathJoinSubstitution([FindPackageShare("moveit_resources_panda_description"), "models"])


    set_gz_path = SetEnvironmentVariable(
        name="GZ_SIM_RESOURCE_PATH",
        value=[
            EnvironmentVariable("GZ_SIM_RESOURCE_PATH", default_value=""),
            os.pathsep,
            models_dir,                              
            os.pathsep,
            FindPackageShare("moveit_resources_panda_description"),            # (optional) also add the package root
        ],
    )

    # Mode switches
    use_gz = PythonExpression(["'", LaunchConfiguration("ros2_control_hardware_type"), "' == 'gz'"])
    use_fake = PythonExpression(["'", LaunchConfiguration("ros2_control_hardware_type"), "' != 'gz'"])

    # --- CLI args ---
    rviz_config_arg = DeclareLaunchArgument(
        "rviz_config", default_value="moveit.rviz", description="RViz configuration file"
    )
    db_arg = DeclareLaunchArgument("db", default_value="False", description="Database flag")
    ros2_control_hardware_type = DeclareLaunchArgument(
        "ros2_control_hardware_type",
        default_value="mock_components",
        description="ROS 2 control hardware interface type -- [mock_components, gz]",
    )

    # --- MoveIt config ---
    moveit_config = (
        MoveItConfigsBuilder("moveit_resources_panda_moveit_config", package_name="moveit_resources_panda_moveit_config")
        .robot_description(
            file_path="config/panda.urdf.xacro",
            mappings={"ros2_control_hardware_type": LaunchConfiguration("ros2_control_hardware_type")},
        )
        .robot_description_semantic(file_path="config/panda.srdf")
        .planning_scene_monitor(publish_robot_description=True, publish_robot_description_semantic=True)
        .trajectory_execution(file_path="config/moveit_controllers.yaml")
        .planning_pipelines(pipelines=["ompl", "chomp", "pilz_industrial_motion_planner", "stomp"])
        .to_moveit_configs()
    )

    # --- Nodes: MoveIt & RViz ---
    move_group_node = Node(
        package="moveit_ros_move_group",
        executable="move_group",
        output="screen",
        parameters=[{"use_sim_time": True},moveit_config.to_dict()],
        arguments=["--ros-args", "--log-level", "info"],
    )

    rviz_config = PathJoinSubstitution([FindPackageShare("moveit_resources_panda_moveit_config"), "launch", LaunchConfiguration("rviz_config")])
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="log",
        arguments=["-d", rviz_config],
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.planning_pipelines,
            moveit_config.robot_description_kinematics,
            moveit_config.joint_limits,
            {"use_sim_time": True},
        ],
    )

    # # --- TF & state publisher ---
    # static_tf_node = Node(
    #     package="tf2_ros",
    #     executable="static_transform_publisher",
    #     name="static_transform_publisher",
    #     output="log",
    #     arguments=["0", "0", "0", "0", "0", "0", "world", "panda_link0"],
    # )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="both",
        parameters=[{"use_sim_time": True},moveit_config.robot_description],
    )

    # --- Common paths ---
    ros2_controllers_path = os.path.join(
        get_package_share_directory("moveit_resources_panda_moveit_config"), "config", "ros2_controllers.yaml"
    )
    world_path = PathJoinSubstitution([FindPackageShare("moveit_resources_panda_description"), "worlds", "empty.world"])

    # ---------------------------
    # Fake mode (no Gazebo)
    # ---------------------------
    ros2_control_node_fake = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[ros2_controllers_path],
        remappings=[("/controller_manager/robot_description", "/robot_description")],
        output="screen",
        condition=IfCondition(use_fake),
    )

    joint_state_broadcaster_spawner_fake = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "-c", "/controller_manager", "--param-file", ros2_controllers_path],
        condition=IfCondition(use_fake),
    )
    panda_arm_controller_spawner_fake = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["panda_arm_controller", "-c", "/controller_manager", "--param-file", ros2_controllers_path],
        condition=IfCondition(use_fake),
    )
    panda_hand_controller_spawner_fake = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["panda_hand_controller", "-c", "/controller_manager", "--param-file", ros2_controllers_path],
        condition=IfCondition(use_fake),
    )

    # ---------------------------
    # Gazebo mode
    # ---------------------------
    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([FindPackageShare("ros_gz_sim"), "launch", "gz_sim.launch.py"])
        ),
        launch_arguments={
            "gz_args": ["-r ", world_path]  # <-- no str()
        }.items(),
        condition=IfCondition(use_gz),
    )

    clock_bridge = Node(
    package="ros_gz_bridge",
    executable="parameter_bridge",
    name="gz_clock_bridge",
    # Use your actual world name here instead of 'default' if different
    arguments=["/world/empty/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock"],
    remappings=[("/world/empty/clock", "/clock")],
    output="screen",
    condition=IfCondition(use_gz),
)

    spawn_entity = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=["-name", "panda", "-topic", "/robot_description"],
        output="screen",
        condition=IfCondition(use_gz),
    )

    joint_state_broadcaster_spawner_gz = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "joint_state_broadcaster", "-c", "/controller_manager",
            "--param-file", ros2_controllers_path
        ],
        condition=IfCondition(use_gz),
    )

    panda_arm_controller_spawner_gz = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "panda_arm_controller", "-c", "/controller_manager",
            "--param-file", ros2_controllers_path
        ],
        condition=IfCondition(use_gz),
    )

    panda_hand_controller_spawner_gz = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "panda_hand_controller", "-c", "/controller_manager",
            "--param-file", ros2_controllers_path
        ],
        condition=IfCondition(use_gz),
    )

    # --- Optional DB ---
    db_config = LaunchConfiguration("db")
    mongodb_server_node = Node(
        package="warehouse_ros_mongo",
        executable="mongo_wrapper_ros.py",
        parameters=[
            {"warehouse_port": 33829},
            {"warehouse_host": "localhost"},
            {"warehouse_plugin": "warehouse_ros_mongo::MongoDatabaseConnection"},
        ],
        output="screen",
        condition=IfCondition(db_config),
    )

    return LaunchDescription(
        [
            rviz_config_arg,
            db_arg,
            ros2_control_hardware_type,
            set_gz_path,
            rviz_node,
            #static_tf_node,
            robot_state_publisher,
            move_group_node,
            # fake-mode chain
            ros2_control_node_fake,
            joint_state_broadcaster_spawner_fake,
            panda_arm_controller_spawner_fake,
            panda_hand_controller_spawner_fake,
            # gazebo-mode chain
            gz_sim,
            clock_bridge,
            spawn_entity,
            joint_state_broadcaster_spawner_gz,
            panda_arm_controller_spawner_gz,
            panda_hand_controller_spawner_gz,
            # optional db
            mongodb_server_node,
        ]
    )