///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2018-2020, University of Edinburgh
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include "state.hpp"
#include "actuation.hpp"
#include "cost.hpp"
#include "crocoddyl/core/actions/diff-lqr.hpp"
#include "crocoddyl/multibody/actions/free-fwddyn.hpp"
#include "crocoddyl/core/numdiff/diff-action.hpp"
#include "crocoddyl/core/utils/exception.hpp"

#ifndef CROCODDYL_ACTION_FACTORY_HPP_
#define CROCODDYL_ACTION_FACTORY_HPP_

namespace crocoddyl {
namespace unittest {

struct DifferentialActionModelTypes {
  enum Type {
    DifferentialActionModelLQR,
    DifferentialActionModelLQRDriftFree,
    DifferentialActionModelFreeFwdDynamics,
    NbDifferentialActionModelTypes
  };
  static std::vector<Type> init_all() {
    std::vector<Type> v;
    v.clear();
    for (int i = 0; i < NbDifferentialActionModelTypes; ++i) {
      v.push_back((Type)i);
    }
    return v;
  }
  static const std::vector<Type> all;
};
const std::vector<DifferentialActionModelTypes::Type> DifferentialActionModelTypes::all(
    DifferentialActionModelTypes::init_all());

std::ostream& operator<<(std::ostream& os, DifferentialActionModelTypes::Type type) {
  switch (type) {
    case DifferentialActionModelTypes::DifferentialActionModelLQR:
      os << "DifferentialActionModelLQR";
      break;
    case DifferentialActionModelTypes::DifferentialActionModelLQRDriftFree:
      os << "DifferentialActionModelLQRDriftFree";
      break;
    case DifferentialActionModelTypes::DifferentialActionModelFreeFwdDynamics:
      os << "DifferentialActionModelFreeFwdDynamics";
      break;
    case DifferentialActionModelTypes::NbDifferentialActionModelTypes:
      os << "NbDifferentialActionModelTypes";
      break;
    default:
      break;
  }
  return os;
}

class DifferentialActionModelFactory {
 public:
  explicit DifferentialActionModelFactory() {}
  ~DifferentialActionModelFactory() {}

  boost::shared_ptr<crocoddyl::DifferentialActionModelAbstract> create(DifferentialActionModelTypes::Type type) {
    boost::shared_ptr<crocoddyl::DifferentialActionModelAbstract> action;
    boost::shared_ptr<crocoddyl::StateMultibody> state;
    boost::shared_ptr<crocoddyl::ActuationModelAbstract> actuation;
    boost::shared_ptr<crocoddyl::CostModelSum> cost;
    switch (type) {
      case DifferentialActionModelTypes::DifferentialActionModelLQR:
        action = boost::make_shared<crocoddyl::DifferentialActionModelLQR>(40, 40, false);
        break;
      case DifferentialActionModelTypes::DifferentialActionModelLQRDriftFree:
        action = boost::make_shared<crocoddyl::DifferentialActionModelLQR>(40, 40, true);
        break;
      case DifferentialActionModelTypes::DifferentialActionModelFreeFwdDynamics:
        state = boost::static_pointer_cast<crocoddyl::StateMultibody>(
            StateModelFactory().create(StateModelTypes::StateMultibody_TalosArm));
        actuation = ActuationModelFactory().create(ActuationModelTypes::ActuationModelFull,
                                                   StateModelTypes::StateMultibody_TalosArm);
        cost = boost::make_shared<crocoddyl::CostModelSum>(state, state->get_nv());
        cost->addCost(
            "state",
            CostModelFactory().create(CostModelTypes::CostModelState, StateModelTypes::StateMultibody_TalosArm,
                                      ActivationModelTypes::ActivationModelQuad),
            1.);
        cost->addCost(
            "control",
            CostModelFactory().create(CostModelTypes::CostModelControl, StateModelTypes::StateMultibody_TalosArm,
                                      ActivationModelTypes::ActivationModelQuad),
            1.);
        cost->addCost("frame",
                      CostModelFactory().create(CostModelTypes::CostModelFramePlacement,
                                                StateModelTypes::StateMultibody_TalosArm,
                                                ActivationModelTypes::ActivationModelQuad),
                      1.);
        action = boost::make_shared<crocoddyl::DifferentialActionModelFreeFwdDynamics>(state, actuation, cost);
        break;
      default:
        throw_pretty(__FILE__ ": Wrong DifferentialActionModelTypes::Type given");
        break;
    }

    return action;
  }

 private:
  std::size_t nq_;
  std::size_t nu_;
};

}  // namespace unittest
}  // namespace crocoddyl

#endif  // CROCODDYL_ACTION_FACTORY_HPP_
