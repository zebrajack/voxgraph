#ifndef VOXGRAPH_FRONTEND_VOXGRAPH_MAPPER_H_
#define VOXGRAPH_FRONTEND_VOXGRAPH_MAPPER_H_

#include <maplab_msgs/OdometryWithImuBiases.h>
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>
#include <std_srvs/Empty.h>
#include <voxblox_msgs/FilePath.h>
#include <memory>
#include <string>
#include "voxgraph/common.h"
#include "voxgraph/frontend/frame_names.h"
#include "voxgraph/frontend/map_tracker/map_tracker.h"
#include "voxgraph/frontend/map_tracker/scan_to_map_registerer.h"
#include "voxgraph/frontend/measurement_processors/gps_processor.h"
#include "voxgraph/frontend/measurement_processors/pointcloud_integrator.h"
#include "voxgraph/frontend/pose_graph_interface/pose_graph_interface.h"
#include "voxgraph/frontend/submap_collection/voxgraph_submap_collection.h"
#include "voxgraph/tools/data_servers/loop_closure_edge_server.h"
#include "voxgraph/tools/data_servers/projected_map_server.h"
#include "voxgraph/tools/data_servers/submap_server.h"
#include "voxgraph/tools/rosbag_helper.h"
#include "voxgraph/tools/visualization/submap_visuals.h"

namespace voxgraph {
class VoxgraphMapper {
 public:
  // Constructor & Destructor
  VoxgraphMapper(const ros::NodeHandle &nh, const ros::NodeHandle &nh_private);
  ~VoxgraphMapper() = default;

  // ROS topic callbacks
  void pointcloudCallback(const sensor_msgs::PointCloud2::Ptr &pointcloud_msg);
  void imuBiasesCallback(const sensor_msgs::Imu::ConstPtr &imu_biases);

  // ROS service callbacks
  bool publishSeparatedMeshCallback(
      std_srvs::Empty::Request &request,     // NOLINT
      std_srvs::Empty::Response &response);  // NOLINT
  bool publishCombinedMeshCallback(
      std_srvs::Empty::Request &request,     // NOLINT
      std_srvs::Empty::Response &response);  // NOLINT
  bool saveToFileCallback(
      voxblox_msgs::FilePath::Request &request,     // NOLINT
      voxblox_msgs::FilePath::Response &response);  // NOLINT

  const VoxgraphSubmapCollection &getSubmapCollection() {
    return *submap_collection_ptr_;
  }

  const PoseGraph::SolverSummaryList &getSolverSummaries() {
    return pose_graph_interface_.getSolverSummaries();
  }

 private:
  // Node handles
  ros::NodeHandle nh_;
  ros::NodeHandle nh_private_;

  // Verbosity and debug mode
  bool verbose_;

  // Flag and helper to automatically pause rosbags during graph optimization
  // NOTE: This is useful when playing bags faster than real-time or when
  //       experimenting with optimization settings
  bool auto_pause_rosbag_;
  RosbagHelper rosbag_helper_;

  // Interaction with ROS
  void subscribeToTopics();
  void advertiseTopics();
  void advertiseServices();
  void getParametersFromRos();

  // ROS topic subscribers
  int subscriber_queue_length_;
  std::string pointcloud_topic_;
  ros::Subscriber pointcloud_subscriber_;
  std::string imu_biases_topic_;
  ros::Subscriber imu_biases_subscriber_;

  // ROS topic publishers
  ros::Publisher separated_mesh_pub_;
  ros::Publisher combined_mesh_pub_;
  ros::Publisher pose_history_pub_;
  ros::Publisher odom_with_imu_biases_pub_;

  // ROS service servers
  ros::ServiceServer publish_separated_mesh_srv_;
  ros::ServiceServer publish_combined_mesh_srv_;
  ros::ServiceServer save_to_file_srv_;
  // TODO(victorr): Add srvs to receive absolute pose and loop closure updates

  // Constraints to be used
  bool registration_constraints_enabled_;
  bool odometry_constraints_enabled_;
  bool height_constraints_enabled_;

  // Instantiate the submap collection
  VoxgraphSubmap::Config submap_config_;
  VoxgraphSubmapCollection::Ptr submap_collection_ptr_;

  // Interface to ease interaction with the pose graph
  PoseGraphInterface pose_graph_interface_;

  // Measurement processors
  PointcloudIntegrator pointcloud_processor_;

  // Map servers, used to share the projected map and submaps with ROS nodes
  ProjectedMapServer projected_map_server_;
  SubmapServer submap_server_;
  LoopClosureEdgeServer loop_closure_edge_server_;

  // Visualization tools
  SubmapVisuals submap_vis_;

  // Map tracker handles the odometry input and refines it using scan-to-map ICP
  MapTracker map_tracker_;

  // TODO(victorr): BOF Move to MapTracker
  // Whether to get the odometry input through TF lookups
  bool use_tf_transforms_;
  bool get_T_base_sensor_from_tfs_;

  // Scan to submap registerer used to refine the odometry estimate,
  // akin to voxblox ICP
  ScanToMapRegisterer scan_to_map_registerer_;
  // TODO(victorr): EOF Move to MapTracker

  BiasVectorType forwarded_accel_bias_ = BiasVectorType::Zero();
  BiasVectorType forwarded_gyro_bias_ = BiasVectorType::Zero();
};
}  // namespace voxgraph

#endif  // VOXGRAPH_FRONTEND_VOXGRAPH_MAPPER_H_
