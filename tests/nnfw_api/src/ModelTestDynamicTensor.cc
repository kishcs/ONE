/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
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

#include <gtest/gtest.h>
#include <nnfw_debug.h>

#include "fixtures.h"
#include "NNPackages.h"

/**
 * @brief Testing the following model:
 *
 * Testing the following model:
 *       #1 = const(value = [-1.5, -1.0, -0.5, 0.5, 1.0, 1.5], shape=[2, 3])
 *       #2 = placeholder (shape = [2])      <-------- this is an input
 *       #3 = reshape(#1, #2)
 *
 * @note Run this test with "cpu" backend
 */
class TestDynamicTensorReshapeModelLoaded
    : public ValidationTestModelLoaded<NNPackages::DYNAMIC_TENSOR_RESHAPE>
{
protected:
  void set_input_output(const std::vector<int> &new_shape, int actual_output_size,
                        std::vector<float> *actual_output)
  {
    NNFW_STATUS res = nnfw_set_input(_session, 0, NNFW_TYPE_TENSOR_INT32, new_shape.data(),
                                     sizeof(int) * new_shape.size());
    ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);

    res = nnfw_set_output(_session, 0, NNFW_TYPE_TENSOR_FLOAT32, actual_output->data(),
                          sizeof(float) * actual_output_size);
    ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);
  }

  void prepare_and_set_input_output(const std::vector<int> &new_shape, int actual_output_size,
                                    std::vector<float> *actual_output)
  {
    ASSERT_EQ(nnfw_set_available_backends(_session, "cpu"), NNFW_STATUS_NO_ERROR);

    NNFW_STATUS res = NNFW_STATUS_ERROR;

    res = nnfw_prepare(_session);
    ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);

    set_input_output(new_shape, actual_output_size, actual_output);
    // real test case should start from calling nnfw_run()
  }

  // call this after calling nnfw_prepare()
  void set_input_output_and_run(const std::vector<int> &new_shape,
                                const std::vector<float> &expected_output, bool no_run_error = true)
  {
    int output_element_num = expected_output.size();
    std::vector<float> actual_output(output_element_num);

    set_input_output(new_shape, output_element_num, &actual_output);

    // Do inference
    NNFW_STATUS res = nnfw_run(_session);

    if (no_run_error)
    {
      ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);

      // output value check
      for (int i = 0; i < expected_output.size(); ++i)
        ASSERT_EQ(expected_output[i], actual_output[i]);
    }
    else
    {
      ASSERT_EQ(res, NNFW_STATUS_ERROR);
    }
  };

  void TearDown() override
  {
    ValidationTestModelLoaded<NNPackages::DYNAMIC_TENSOR_RESHAPE>::TearDown();
  }
};

TEST_F(TestDynamicTensorReshapeModelLoaded, reshape_to_3x2)
{
  const std::vector<int> new_shape = {3, 2};
  const std::vector<float> expected = {-1.5, -1.0, -0.5, 0.5, 1.0, 1.5};
  std::vector<float> actual_output(expected.size());

  prepare_and_set_input_output(new_shape, expected.size(), &actual_output);

  // Do inference
  NNFW_STATUS res = nnfw_run(_session);
  ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);

  // output value check
  for (int i = 0; i < expected.size(); ++i)
    ASSERT_EQ(expected[i], actual_output[i]);
}

/**
 * @brief Negative test.
 *        Reshape's first input has 6 values but trying to reshaping to [3, 3]
 */
TEST_F(TestDynamicTensorReshapeModelLoaded, neg_reshape_to_wrong_3x3)
{
  const std::vector<int> wrong_shape = {3, 3}; // wrong shape input
  const int actual_element_num = 9;            // whatever number
  std::vector<float> actual_output(9);         // whatever size

  prepare_and_set_input_output(wrong_shape, actual_element_num, &actual_output);

  // Do inference
  NNFW_STATUS res = nnfw_run(_session);
  ASSERT_EQ(res, NNFW_STATUS_ERROR); // run should fail
}

TEST_F(TestDynamicTensorReshapeModelLoaded, reshape_multiple_executions)
{
  ASSERT_EQ(nnfw_set_available_backends(_session, "cpu"), NNFW_STATUS_NO_ERROR);

  NNFW_STATUS res = nnfw_prepare(_session);
  ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);

  std::vector<int> new_shape;
  std::vector<float> expected = {-1.5, -1.0, -0.5, 0.5, 1.0, 1.5};

  // let's call multiple times
  new_shape = {3, 2};
  set_input_output_and_run(new_shape, expected);

  new_shape = {1, 6};
  set_input_output_and_run(new_shape, expected);

  new_shape = {6, 1};
  set_input_output_and_run(new_shape, expected);
}

TEST_F(TestDynamicTensorReshapeModelLoaded, neg_reshape_multiple_executions)
{
  ASSERT_EQ(nnfw_set_available_backends(_session, "cpu"), NNFW_STATUS_NO_ERROR);

  NNFW_STATUS res = nnfw_prepare(_session);
  ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);

  std::vector<int> new_shape;
  std::vector<float> expected = {-1.5, -1.0, -0.5, 0.5, 1.0, 1.5};

  // let's call multiple times including the second nnfw_run() to fail
  new_shape = {3, 2};
  set_input_output_and_run(new_shape, expected);

  new_shape = {1, 100};                                 // wrong shape
  set_input_output_and_run(new_shape, expected, false); // Run will fail

  // next run should succeed
  new_shape = {6, 1};
  set_input_output_and_run(new_shape, expected);
}

// Unknown Dimension Test
//

class TestInputUnknownDimInputConcatModelLoaded
    : public ValidationTestModelLoaded<NNPackages::UNKNOWN_DIM_INPUT_CONCAT>
{
protected:
  void prepare_apply_set_input_output(const std::vector<float> &input0,
                                      const std::vector<float> &input1,
                                      std::vector<float> *actual_output, nnfw_tensorinfo input0_ti)
  {
    ASSERT_EQ(nnfw_prepare(_session), NNFW_STATUS_NO_ERROR);
    ASSERT_EQ(nnfw_apply_tensorinfo(_session, 0, input0_ti), NNFW_STATUS_NO_ERROR);

    ASSERT_EQ(nnfw_set_input(_session, 0, NNFW_TYPE_TENSOR_FLOAT32, input0.data(),
                             sizeof(float) * input0.size()),
              NNFW_STATUS_NO_ERROR);
    ASSERT_EQ(nnfw_set_input(_session, 1, NNFW_TYPE_TENSOR_FLOAT32, input1.data(),
                             sizeof(float) * input1.size()),
              NNFW_STATUS_NO_ERROR);

    ASSERT_EQ(nnfw_set_output(_session, 0, NNFW_TYPE_TENSOR_FLOAT32, actual_output->data(),
                              sizeof(float) * actual_output->size()),
              NNFW_STATUS_NO_ERROR);
  }
};

/**
 * @brief Testing the following model:
 *
 *        #0 = placeholder([None, None])
 *        #1 = placeholder([2, 3])
 *        #2 = concat (#0, #1, axis=0)
 *
 *        Calling sequence:
 *        - nnfw_prepare()
 *        - nnfw_apply_tensor_info(#0, [1, 3])
 *        - nnfw_set_input()
 *        - nnfw_run()
 *
 * @note Run this test with "cpu" backend
 */
TEST_F(TestInputUnknownDimInputConcatModelLoaded, concat_input0_to_2x3)
{
  ASSERT_EQ(nnfw_set_available_backends(_session, "cpu"), NNFW_STATUS_NO_ERROR);

  const std::vector<float> input0 = {1, 2, 3};          // of shape [1, 3]
  const std::vector<float> input1 = {4, 5, 6, 7, 8, 9}; // of shape [2, 3]

  const std::vector<float> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<float> actual_output(expected.size());

  // input reshaping to [1, 3]
  nnfw_tensorinfo ti;
  {
    ti.dtype = NNFW_TYPE_TENSOR_FLOAT32;
    ti.rank = 2;
    ti.dims[0] = 1;
    ti.dims[1] = 3;
  }

  prepare_apply_set_input_output(input0, input1, &actual_output, ti);

  // Do inference
  NNFW_STATUS res = nnfw_run(_session);
  ASSERT_EQ(res, NNFW_STATUS_NO_ERROR);

  // output value check
  for (int i = 0; i < expected.size(); ++i)
    ASSERT_EQ(expected[i], actual_output[i]);
}

/**
 * @brief Negative Test: Testing the following model:
 *
 *        #0 = placeholder([None, None])
 *        #1 = placeholder([2, 3])
 *        #2 = concat (#0, #1, axis=0)
 *
 *        Calling sequence:
 *        - nnfw_prepare()
 *        - nnfw_apply_tensor_info(#0, [3, 1]) ---> input shape is not matched for concat to work
 *        - nnfw_set_input()
 *        - nnfw_run()
 *
 * @note Run this test with "cpu" backend and "linear" executor
 */
TEST_F(TestInputUnknownDimInputConcatModelLoaded, neg_concat_input0_to_wrong_shape)
{
  ASSERT_EQ(nnfw_set_available_backends(_session, "cpu"), NNFW_STATUS_NO_ERROR);

  const std::vector<float> input0 = {1, 2, 3};          // of shape [3, 1], wrong shape
  const std::vector<float> input1 = {4, 5, 6, 7, 8, 9}; // of shape [2, 3]

  std::vector<float> actual_output(100); // whatever size

  // input reshaping to [3, 1]
  nnfw_tensorinfo ti;
  {
    ti.dtype = NNFW_TYPE_TENSOR_FLOAT32;
    ti.rank = 2;
    ti.dims[0] = 3;
    ti.dims[1] = 1;
  }

  prepare_apply_set_input_output(input0, input1, &actual_output, ti);

  // Do inference
  NNFW_STATUS res = nnfw_run(_session);
  ASSERT_EQ(res, NNFW_STATUS_ERROR);
}

// Complex Model Test
