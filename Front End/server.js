//Alex Cen Feng
/*******************************************************
 * server.js
 *******************************************************/
const dgram = require('dgram');
const sqlite3 = require('sqlite3').verbose();
const express = require('express');
const path = require('path');
const http = require('http');
const { Server } = require('socket.io');

// 1) Initialize or create the database
const db = new sqlite3.Database('scores.db', (err) => {
  if (err) {
    return console.error('Error opening SQLite DB:', err.message);
  }
  console.log('Connected to local SQLite database (scores.db).');
});

// Create table if it doesn't exist
db.run(`
  CREATE TABLE IF NOT EXISTS scores (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    playerName TEXT,
    fastestLap TEXT,
    raceTime TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
  )
`, (err) => {
  if (err) {
    console.error('Error creating scores table:', err.message);
  }
});

// 2) Set up the HTTP (Express) server
const app = express();
const httpPort = 3000;
app.use(express.json());
app.use(express.static(path.join(__dirname)));

// Leaderboard data route
app.get('/leaderboardData', (req, res) => {
  db.all(`SELECT * FROM scores ORDER BY id DESC`, [], (err, rows) => {
    if (err) {
      console.error('Error querying leaderboard:', err);
      return res.json([]);
    }
    res.json(rows);
  });
});

// Save name route (inserts new row with just name, placeholders for times)
app.post('/saveName', (req, res) => {
  const { playerName } = req.body;
  if (!playerName) {
    return res.status(400).json({ error: 'No playerName provided' });
  }

  // Insert a placeholder row
  db.run(
    `INSERT INTO scores (playerName, fastestLap, raceTime) VALUES (?, ?, ?)`,
    [playerName, '', ''],
    function(err) {
      if (err) {
        console.error('Error inserting name into scores:', err);
        return res.status(500).json({ error: 'DB insert error' });
      }
      // Return the row's ID so the client can update it later
      res.json({ id: this.lastID });
    }
  );
});

// Update final result route
app.post('/saveResult', (req, res) => {
  const { id, fastestLap, totalTime } = req.body;
  if (!id) {
    return res.status(400).json({ error: 'No ID provided' });
  }
  db.run(
    `UPDATE scores SET fastestLap = ?, raceTime = ? WHERE id = ?`,
    [fastestLap, totalTime, id],
    function(err) {
      if (err) {
        console.error('Error updating final result:', err);
        return res.status(500).json({ error: 'DB update error' });
      }
      return res.json({ success: true });
    }
  );
});

// Test route
app.get('/test', (req, res) => {
  res.send('Server is running!');
});

// Store the last known gyro reading
let lastGyroReading = { x: 0, y: 0, z: 0 };
app.get('/latestGyro', (req, res) => {
  res.json(lastGyroReading);
});

// Create an HTTP server, attach Socket.IO
const server = http.createServer(app);
const io = new Server(server);

// Start the HTTP + Socket.IO server
server.listen(httpPort, () => {
  console.log(`HTTP + Socket.IO server listening on http://localhost:${httpPort}`);
});

// 3) Set up the UDP server
const udpPort = 41234;
const udpServer = dgram.createSocket('udp4');

udpServer.on('message', (msg, rinfo) => {
  console.log(`UDP message from ${rinfo.address}:${rinfo.port} -> ${msg}`);

  try {
    const data = JSON.parse(msg);

    if (data.type === 'gyro') {
      // Update global lastGyroReading if needed
      lastGyroReading.x = data.x;
      lastGyroReading.y = data.y;
      lastGyroReading.z = data.z;
    }
    else if (data.type === 'button_press') {
      // BROADCAST a “button_press” event via Socket.IO
      io.emit('button_press', { message: data.message });
    }
    else if (data.playerName && data.fastestLap && data.raceTime) {
      // Insert new score (example usage)
      db.run(
        `INSERT INTO scores (playerName, fastestLap, raceTime) VALUES (?, ?, ?)`,
        [data.playerName, data.fastestLap, data.raceTime],
        (err) => {
          if (err) {
            console.error('Error inserting data into scores table:', err.message);
          } else {
            console.log('Successfully inserted new score into DB via UDP.');
          }
        }
      );
    }
  } catch (error) {
    console.error('Failed to parse UDP message as JSON:', error.message);
  }
});

udpServer.on('error', (err) => {
  console.log(`UDP server error:\n${err.stack}`);
  udpServer.close();
});

udpServer.bind(udpPort, () => {
  console.log(`UDP server listening on port ${udpPort}`);
});
