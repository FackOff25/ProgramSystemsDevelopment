import os

from flask import Flask, render_template, json
from db_work import call_proc, select, select_dict
from sql_provider import SQLProvider

from geometry import Point, Element
import plotter

app = Flask(__name__)

app.config['db_config'] = json.load(open('data_files/dbconfig.json'))

provider = SQLProvider(os.path.join(os.path.dirname(__file__), 'sql'))

@app.route('/', methods=['GET', 'POST'])
def main_page():
    return render_template("page.html")

if __name__ == '__main__':
    sql = provider.get('get_elements.sql')
    elements_list = select(app.config['db_config'], sql)[0]
    elements_dict = {}

    for element in elements_list:
        sql = provider.get('get_node.sql', id=element[1])
        db_point = select_dict(app.config['db_config'], sql)[0]
        point1 = Point(db_point['id'], db_point['x'], db_point['y'])

        sql = provider.get('get_node.sql', id=element[2])
        db_point = select_dict(app.config['db_config'], sql)[0]
        point2 = Point(db_point['id'], db_point['x'], db_point['y'])

        sql = provider.get('get_node.sql', id=element[3])
        db_point = select_dict(app.config['db_config'], sql)[0]
        point3 = Point(db_point['id'], db_point['x'], db_point['y'])

        cur_element = Element(element[0], point1, point2, point3)
        elements_dict[element[0]] = cur_element.get_perimeter()

    plotter.make_plot(elements_dict, "plot.png")
    app.run(host='127.0.0.1', port=8000)
