#!/usr/bin/python

import pandas as pd
import matplotlib.pyplot as plt
import re
import numpy as np

def open_data():
    edf = pd.read_csv('events.csv')

    drogue_time = None
    main_time = None

    drogue_row = edf.loc[edf['Event'] == 'DROGUE']
    main_row = edf.loc[edf['Event'] == 'MAIN']

    if len(drogue_row) != 0:
        drogue_time = float(drogue_row.iloc[0].loc['TimeTicks'])
    
    if len(main_row) != 0:
        main_time = float(main_row.iloc[0].loc['TimeTicks'])

    df = pd.read_csv('temp_pressure.csv')
    df['TimeMS'] = df['TimeTicks']

    return df, drogue_time, main_time

def plot_data(df, pyro_event_time, chute_event_time):
    fig, ax1 = plt.subplots()

    ax1.set_title('Test Launch: Pressure vs Time')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Pressure (Pa)', color='tab:blue')
    ax1.scatter((df['TimeMS'] / 1000.0), df['PressurePA'], color='tab:blue', marker='.')
    ax1.tick_params(axis='y', labelcolor='tab:blue')

    if pyro_event_time is not None:
        pyro_time = pyro_event_time / 1000
        ax1.axvline(x=pyro_time, color='tab:orange', linestyle='--', label=f"Firing drogue at {pyro_time:.2f}s")
        ax1.legend()

    if chute_event_time is not None:
        chute_time = chute_event_time / 1000
        ax1.axvline(x=chute_time, color='tab:green', linestyle='--', label=f"Firing chute at {chute_time:.2f}s")
        ax1.legend()

    ax1.grid(True, linestyle='--', color='grey', alpha=0.6)

    # Custom x-axis ticks and labels
    xmin, xmax, N = 0, max(df['TimeMS']) / 1000, 10
    custom_ticks = np.linspace(xmin, xmax, N, dtype=float)
    rounded_ticks = np.round(custom_ticks * 2) / 2  # Round to the nearest 0.5
    ax1.set_xticks(rounded_ticks)
    ax1.set_xticklabels(rounded_ticks)

    fig.tight_layout()
    plt.show()

data_df, pyro_event_time, chute_event_time = open_data()
plot_data(data_df, pyro_event_time, chute_event_time)
