import os

import numpy as np
import matplotlib.pyplot as plt

def make_plot(dict, filename):
    els = list(dict.keys())
    values = list(dict.values())

    colors = ['red', 'blue', 'green']

    fig = plt.figure(figsize = (10, 5))

    plt.bar(els, values, color=colors)
    plt.xticks(els)
    # plt.yticks(values)

    plt.xlabel("Элементы")
    plt.ylabel("Периметр")

    plt.savefig(os.path.join(os.path.dirname(__file__), 'static', filename))
