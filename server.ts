import express from "express";
import { createServer as createViteServer } from "vite";
import path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

async function startServer() {
  const app = express();
  const PORT = 3000;

  // Mock metrics for the 100-stream system
  const getMockMetrics = () => {
    return {
      timestamp: new Date().toISOString(),
      streams: Array.from({ length: 100 }, (_, i) => ({
        id: i,
        status: Math.random() > 0.02 ? "active" : "error",
        fps: 24.5 + Math.random() * 1.5,
        latency: 3.8 + Math.random() * 0.8,
        gpuId: i < 50 ? 0 : 1,
      })),
      gpus: [
        {
          id: 0,
          model: "RTX 4090",
          utilization: 72 + Math.random() * 10,
          vramUsed: 18.2 + Math.random() * 1,
          vramTotal: 48,
          temp: 62 + Math.random() * 4,
          power: 285 + Math.random() * 30,
        },
        {
          id: 1,
          model: "RTX 4090",
          utilization: 68 + Math.random() * 10,
          vramUsed: 17.5 + Math.random() * 1,
          vramTotal: 48,
          temp: 59 + Math.random() * 4,
          power: 270 + Math.random() * 30,
        },
      ],
      system: {
        cpuUsage: 8 + Math.random() * 4,
        ramUsed: 42,
        ramTotal: 512,
      }
    };
  };

  app.get("/api/metrics", (req, res) => {
    res.json(getMockMetrics());
  });

  app.post("/api/benchmark", (req, res) => {
    // Simulate capacity calculation for 3 FPS sampling
    setTimeout(() => {
      res.json({
        maxStreams: 500,
        bottleneck: "CPU_NETWORK_IO_LIMIT",
        recommendation: "Increase CPU core count or use multiple network interfaces to handle 500+ concurrent RTSP connections.",
        details: {
          vramLimit: 960,
          nvdecLimit: 666,
          inferenceLimit: 555
        }
      });
    }, 2000);
  });

  app.post("/api/generate-engine", (req, res) => {
    // Simulate YOLO engine generation
    let progress = 0;
    const interval = setInterval(() => {
      progress += 10;
      if (progress >= 100) {
        clearInterval(interval);
      }
    }, 500);

    res.json({ status: "started", model: "YOLOv8n-Person" });
  });

  // Vite middleware for development
  if (process.env.NODE_ENV !== "production") {
    const vite = await createViteServer({
      server: { middlewareMode: true },
      appType: "spa",
    });
    app.use(vite.middlewares);
  } else {
    const distPath = path.join(process.cwd(), "dist");
    app.use(express.static(distPath));
    app.get("*", (req, res) => {
      res.sendFile(path.join(distPath, "index.html"));
    });
  }

  app.listen(PORT, "0.0.0.0", () => {
    console.log(`Server running on http://localhost:${PORT}`);
  });
}

startServer();
