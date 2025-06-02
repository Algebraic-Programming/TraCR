#   Copyright 2025 Huawei Technologies Co., Ltd.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import re
import glob
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt

matplotlib.use('Agg')


def get_event_values(file_path, event_type):
    values = {}
    current_type = None
    reading_values = False

    # Open and read through the file line-by-line
    with open(file_path, 'r') as f:
        for line in f:
            # Look for an EVENT_TYPE line and get its value
            event_match = re.match(r"EVENT_TYPE", line)
            if event_match:
                current_type = int(next(f).split()[1])

            # Check if this EVENT_TYPE matches the user's input
            if current_type == event_type:
                # Start reading values when we hit the VALUES section
                if 'VALUES' in line:
                    reading_values = True
                    continue  # Skip the VALUES line itself

                # Collect values as long as we're in the VALUES section
                if reading_values:
                    value_match = re.match(r"\d+\s+(.*)", line)
                    if value_match:
                        marker_value = int(line.split()[0])
                        marker_title = value_match.group(1)
                        values[marker_value] = marker_title
                    else:
                        # Stop reading if a non-value line is encountered
                        break
    return values

def main():
    """This script is meant for extracting the information of a ovni trace
    and creating it into a matplotlib bar plot.

    Before running this script, load the ovni trace (cpu.prv or thread.prv) in Paraver and right click on the trace window -> Save -> Text...
    and store the CSV file into the ovni folder.

    Run this script with: python3 main.py path/to/ovni/ *optional marker type*

    - path/to/ovni/: is the path to the emulated ovni folder with the stored CSV file in it
    - *optional marker type*: (optional) you can choose which type of markers to plot (default none)
    """

    if len(sys.argv) < 2:
        raise RuntimeError("Please provide the ovni folder path you want to plot")
    folder_path = sys.argv[1]
    
    pcf_file = os.path.join(folder_path, 'thread.pcf')
    
    # get the marker titles if existing else marker values will be used
    if os.path.exists(pcf_file):

        # if specified use the wanted type of markers. Default: 100 (i.e. 0 in ovni)
        if len(sys.argv) == 3:
            event_type = int(sys.argv[2])
        else:
            event_type = 100    # default event type

        event_values = get_event_values(pcf_file, event_type)
        
        # manually add the idling marker as this is default in ovni
        event_values[0] = "Idling"
    else:
        event_values = {}
    

    generated_csv_file = glob.glob(os.path.join(folder_path, '*.csv'))[0]

    # load csv file
    df = pd.read_csv(generated_csv_file, sep='\s+', header=None, names=['ThreadID', 'StartTime', 'Duration', 'Marker'])

    # Group by both ThreadID and Marker, and sum the Duration for each group
    df = df.groupby(['ThreadID', 'Marker'])['Duration'].sum().unstack(fill_value=0)

    # hardcoded the first thread away (will be removed later)
    df = df.iloc[1:,:]
    
    # remove columns full of zeros if existing
    df = df.loc[:, (df != 0.0).any(axis=0)]

    # group all threads into one
    # df = df.sum() # continue here

    # us -> s
    df /= 1e6

    print(df)

    # Plot grouped time data
    ax = df.plot(kind='bar', stacked=False, figsize=(10, 6), width=0.8, zorder=2)
    plt.xlabel("Thread IDs with Markers")
    plt.ylabel("Duration (s)")
    # plt.title("Duration by Thread ID")
    plt.title("Fibonacci(25)")
    plt.xticks(rotation=45, ha="right")
    plt.legend(title="Marker", bbox_to_anchor=(1.05, 1), loc="upper left")

    # update the legend if markers where found
    if len(event_values) > 0:
        new_labels = [event_values.get(int(float(label))) for label in ax.get_legend_handles_labels()[1]]
        ax.legend(title="Marker", labels=new_labels, bbox_to_anchor=(1.05, 1), loc="upper left")
    
    # Remove plot box and add horizontal grid lines
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_visible(False)
    ax.spines['bottom'].set_visible(False)
    ax.yaxis.grid(True, linestyle='-', alpha=0.8, zorder=1)  # Add horizontal grid lines

    plt.tight_layout()

    filename = sys.argv[3] if len(sys.argv) == 4 else "plot.png"
    plt.savefig(filename)


if __name__ == "__main__":
    main()