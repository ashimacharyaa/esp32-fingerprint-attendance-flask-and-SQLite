from flask import (
    Flask,
    request,
    redirect,
    url_for,
    render_template_string,
    Response
)

import sqlite3
from datetime import datetime
import csv
import io


app = Flask(__name__)

DATABASE = "attendance.db"


# =========================================================
# DATABASE
# =========================================================

def get_db():
    conn = sqlite3.connect(DATABASE)
    conn.row_factory = sqlite3.Row
    return conn


def init_db():
    db = get_db()

    db.execute("""
        CREATE TABLE IF NOT EXISTS employees (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL
        )
    """)

    db.execute("""
        CREATE TABLE IF NOT EXISTS attendance (
            record_id INTEGER PRIMARY KEY AUTOINCREMENT,
            employee_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            attendance_date TEXT NOT NULL,
            attendance_time TEXT NOT NULL,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    """)

    db.execute("""
        INSERT OR IGNORE INTO employees (id, name)
        VALUES (?, ?)
    """, (20, "Ashim"))

    db.commit()
    db.close()


init_db()


# =========================================================
# HTML PAGE
# =========================================================

PAGE = """
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">

    <meta
        name="viewport"
        content="width=device-width, initial-scale=1"
    >

    {{ refresh|safe }}

    <title>Fingerprint Attendance Dashboard</title>

    <style>

        body {
            margin: 0;
            font-family: Arial, sans-serif;
            background: #f4f6f8;
        }

        .header {
            background: #1f2937;
            color: white;
            padding: 20px 30px;
        }

        .header h1 {
            margin: 0;
        }

        .container {
            max-width: 1100px;
            margin: 30px auto;
            padding: 0 20px;
        }

        .nav {
            margin-bottom: 20px;
        }

        .nav a {
            display: inline-block;
            padding: 10px 15px;
            margin-right: 8px;
            text-decoration: none;
            background: #1f2937;
            color: white;
            border-radius: 6px;
        }

        .cards {
            display: flex;
            gap: 20px;
            flex-wrap: wrap;
        }

        .card {
            background: white;
            padding: 20px;
            border-radius: 10px;
            min-width: 200px;
            flex: 1;
            box-shadow: 0 2px 8px #ddd;
        }

        .big {
            font-size: 32px;
            font-weight: bold;
            margin-top: 10px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            background: white;
            margin-top: 20px;
        }

        th,
        td {
            padding: 12px;
            border-bottom: 1px solid #ddd;
            text-align: left;
        }

        th {
            background: #e5e7eb;
        }

        form {
            background: white;
            padding: 20px;
            border-radius: 10px;
            margin-top: 20px;
        }

        input {
            padding: 10px;
            margin: 5px;
        }

        button {
            padding: 10px 15px;
            cursor: pointer;
        }

        .message {
            background: #e8f5e9;
            padding: 12px;
            border-radius: 6px;
            margin-bottom: 15px;
        }

    </style>

</head>

<body>

<div class="header">
    <h1>Fingerprint Attendance Dashboard</h1>
</div>

<div class="container">

    <div class="nav">
        <a href="/">Dashboard</a>
        <a href="/employees">Employees</a>
        <a href="/export">Download CSV</a>
    </div>

    {{ content|safe }}

</div>

</body>

</html>
"""


# =========================================================
# DASHBOARD
# =========================================================

@app.route("/")
def dashboard():

    db = get_db()

    today = datetime.now().strftime("%Y-%m-%d")

    rows = db.execute("""
        SELECT
            record_id,
            employee_id,
            name,
            attendance_date,
            attendance_time
        FROM attendance
        ORDER BY record_id DESC
        LIMIT 100
    """).fetchall()

    employee_count = db.execute("""
        SELECT COUNT(*) AS total
        FROM employees
    """).fetchone()["total"]

    today_count = db.execute("""
        SELECT COUNT(DISTINCT employee_id) AS total
        FROM attendance
        WHERE attendance_date = ?
    """, (today,)).fetchone()["total"]

    db.close()

    html = f"""
    <div class="cards">

        <div class="card">
            <div>Registered Employees</div>
            <div class="big">{employee_count}</div>
        </div>

        <div class="card">
            <div>Present Today</div>
            <div class="big">{today_count}</div>
        </div>

        <div class="card">
            <div>Date</div>
            <div class="big">{today}</div>
        </div>

    </div>

    <h2>Recent Attendance</h2>

    <table>

        <tr>
            <th>ID</th>
            <th>Name</th>
            <th>Date</th>
            <th>Time</th>
        </tr>
    """

    for row in rows:
        html += f"""
        <tr>
            <td>{row['employee_id']}</td>
            <td>{row['name']}</td>
            <td>{row['attendance_date']}</td>
            <td>{row['attendance_time']}</td>
        </tr>
        """

    html += "</table>"

    return render_template_string(
        PAGE,
        content=html,
        refresh='<meta http-equiv="refresh" content="3">'
    )


# =========================================================
# EMPLOYEES PAGE
# =========================================================

@app.route("/employees")
def employees():

    db = get_db()

    people = db.execute("""
        SELECT id, name
        FROM employees
        ORDER BY id
    """).fetchall()

    db.close()

    html = """
    <h2>Employees</h2>

    <form
        method="POST"
        action="/employees/add"
    >

        <h3>Add / Update Employee</h3>

        <label>Fingerprint ID:</label>

        <input
            type="number"
            name="id"
            min="1"
            max="300"
            required
        >

        <label>Name:</label>

        <input
            type="text"
            name="name"
            required
        >

        <button type="submit">
            Save Employee
        </button>

    </form>

    <table>

        <tr>
            <th>Fingerprint ID</th>
            <th>Name</th>
        </tr>
    """

    for person in people:
        html += f"""
        <tr>
            <td>{person['id']}</td>
            <td>{person['name']}</td>
        </tr>
        """

    html += "</table>"

    return render_template_string(
        PAGE,
        content=html,
        refresh=""
    )


# =========================================================
# ADD / UPDATE EMPLOYEE
# =========================================================

@app.route(
    "/employees/add",
    methods=["POST"]
)
def add_employee():

    employee_id = request.form.get("id")
    name = request.form.get("name")

    if not employee_id or not name:
        return "ID and Name are required", 400

    employee_id = int(employee_id)
    name = name.strip()

    db = get_db()

    db.execute("""
        INSERT INTO employees (
            id,
            name
        )
        VALUES (?, ?)

        ON CONFLICT(id)
        DO UPDATE SET
            name = excluded.name
    """, (
        employee_id,
        name
    ))

    db.commit()
    db.close()

    print(
        f"Employee saved: "
        f"ID={employee_id}, "
        f"Name={name}"
    )

    return redirect(
        url_for("employees")
    )


# =========================================================
# ESP32 ATTENDANCE API
# =========================================================

@app.route(
    "/api/attendance",
    methods=["POST"]
)
def api_attendance():

    fingerprint_id = request.form.get("id")

    if not fingerprint_id:
        return "INVALID", 400

    db = get_db()

    employee = db.execute("""
        SELECT
            id,
            name
        FROM employees
        WHERE id = ?
    """, (
        fingerprint_id,
    )).fetchone()

    if employee is None:
        db.close()
        return "UNKNOWN", 404

    now = datetime.now()

    attendance_date = now.strftime("%Y-%m-%d")
    attendance_time = now.strftime("%H:%M:%S")

    db.execute("""
        INSERT INTO attendance (
            employee_id,
            name,
            attendance_date,
            attendance_time
        )
        VALUES (?, ?, ?, ?)
    """, (
        fingerprint_id,
        employee["name"],
        attendance_date,
        attendance_time
    ))

    db.commit()

    name = employee["name"]

    db.close()

    print(
        f"Attendance saved: "
        f"ID={fingerprint_id}, "
        f"Name={name}, "
        f"Date={attendance_date}, "
        f"Time={attendance_time}"
    )

    return name, 200


# =========================================================
# CSV EXPORT
# =========================================================

@app.route("/export")
def export_csv():

    db = get_db()

    rows = db.execute("""
        SELECT
            employee_id,
            name,
            attendance_date,
            attendance_time
        FROM attendance
        ORDER BY record_id DESC
    """).fetchall()

    db.close()

    output = io.StringIO()

    writer = csv.writer(output)

    writer.writerow([
        "ID",
        "Name",
        "Date",
        "Time"
    ])

    for row in rows:

        writer.writerow([
            row["employee_id"],
            row["name"],
            row["attendance_date"],
            row["attendance_time"]
        ])

    return Response(
        output.getvalue(),
        mimetype="text/csv",
        headers={
            "Content-Disposition":
            "attachment; filename=attendance.csv"
        }
    )


# =========================================================
# START SERVER
# =========================================================

if __name__ == "__main__":

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )