/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MIR_INTERPRETER_
#define _MIR_INTERPRETER_

#include "mir/Visitor.h"
#include "mir/Operation.h"
#include "mir/TensorVariant.h"
#include <unordered_map>
#include <vector>

namespace mir_interpreter
{

class MIRInterpreter : public mir::Visitor
{
public:
  explicit MIRInterpreter() = default;

  ~MIRInterpreter() override = default;

  /// @brief Set tensor to the interpreter environment.
  void setTensor(const mir::Operation::Output *output, mir::TensorVariant tensor);

  /// @brief Get tensor from the interpreter environment.
  const mir::TensorVariant &getTensor(const mir::Operation::Output *) const;

  void visit(mir::ops::AddOp &op) override;
  void visit(mir::ops::AbsOp &op) override;
  void visit(mir::ops::AvgPool2DOp &op) override;
  void visit(mir::ops::CappedReluOp &op) override;
  void visit(mir::ops::ConcatOp &op) override;
  void visit(mir::ops::ConstantOp &op) override;
  void visit(mir::ops::Conv2DOp &op) override;
  void visit(mir::ops::DeConv2DOp &op) override;
  void visit(mir::ops::DepthwiseConv2DOp &op) override;
  void visit(mir::ops::DequantizeOp &op) override;
  void visit(mir::ops::DivOp &op) override;
  void visit(mir::ops::EluOp &op) override;
  void visit(mir::ops::EqualOp &op) override;
  void visit(mir::ops::FullyConnectedOp &op) override;
  void visit(mir::ops::GatherOp &op) override;
  void visit(mir::ops::GreaterOp &op) override;
  void visit(mir::ops::HardSwishOp &op) override;
  void visit(mir::ops::InputOp &op) override;
  void visit(mir::ops::LeakyReluOp &op) override;
  void visit(mir::ops::LessOp &op) override;
  void visit(mir::ops::MaxOp &op) override;
  void visit(mir::ops::MaxPool2DOp &op) override;
  void visit(mir::ops::MulOp &op) override;
  void visit(mir::ops::OutputOp &op) override;
  void visit(mir::ops::PadOp &op) override;
  void visit(mir::ops::QuantizeOp &op) override;
  void visit(mir::ops::ReduceMeanOp &op) override;
  void visit(mir::ops::ReluOp &op) override;
  void visit(mir::ops::ReshapeOp &op) override;
  void visit(mir::ops::ResizeOp &op) override;
  void visit(mir::ops::SigmoidOp &op) override;
  void visit(mir::ops::SliceOp &op) override;
  void visit(mir::ops::SoftmaxOp &op) override;
  void visit(mir::ops::SqrtOp &op) override;
  void visit(mir::ops::SqueezeOp &op) override;
  void visit(mir::ops::SubOp &op) override;
  void visit(mir::ops::TanhOp &op) override;
  void visit(mir::ops::TransposeOp &op) override;
  void visit(mir::ops::BroadcastOp &op) override;

protected:
  void visit_fallback(mir::Operation &op) override;

private:
  mir::TensorVariant &allocateTensor(const mir::Operation::Output *output);

  /// @brief Gets the computed inputs for the operation.
  std::vector<std::reference_wrapper<const mir::TensorVariant>>
  getInputTensors(const mir::Operation &op);

  std::vector<std::reference_wrapper<mir::TensorVariant>>
  allocateOutputTensors(const mir::Operation &op);

  /// @brief Mapping of operation outputs to corresponding tensors.
  std::unordered_map<const mir::Operation::Output *, mir::TensorVariant> _tensors;
};

} // namespace mir_interpreter

#endif // _MIR_INTERPRETER_
