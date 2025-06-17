"""
    Copyright 2025 Huawei Technologies Co., Ltd.
 
  Licensed under the Apache License, Version 2.0 (the "License")
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
 
      http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
"""

from tracr import *

"""
 Basic c code to demonstrate TraCR's VMAKERS
 This example uses the VMARKER SET method
 you can create a nice gradient plot in Paraver with it
"""
def main():
  # Initialize TraCR
  INSTRUMENTATION_START()

  # use flag == 1 for push/pop and flag == 0 for the set method
  flag = 0
  INSTRUMENTATION_VMARKER_TYPE(flag, "Simple VMarker Example")

  n = 150
  for i in range(1, n+1):
    INSTRUMENTATION_VMARKER_SET(i)
    print(i, end=' ')

  # Optional: This marker pushes the int64_t max value. Can be used to indicate the ending.
  INSTRUMENTATION_VMARKER_RESET()  

  # TraCR finished
  INSTRUMENTATION_END()

if __name__ == "__main__":
  main()