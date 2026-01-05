import psutil
from flask import Flask, jsonify

app = Flask(__name__)

def get_pc_stats():
    stats = {}

    stats['cpu_percent'] = psutil.cpu_percent(interval=1)

    virtual_mem = psutil.virtual_memory()
    stats['memory_total'] = virtual_mem.total
    stats['memory_used'] = virtual_mem.used
    stats['memory_percent'] = virtual_mem.percent

    disk_usage = psutil.disk_usage('/')
    stats['disk_total'] = disk_usage.total
    stats['disk_used'] = disk_usage.used
    stats['disk_percent'] = disk_usage.percent

    print(stats)

    return stats

@app.route('/pc_stats')
def pc_stats_endpoint():
    return jsonify(get_pc_stats())

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5555)
