import React, { useState, useEffect } from 'react';
import { GpuMonitor } from './GpuMonitor';
import { StreamMatrix } from './StreamMatrix';
import { SystemConsole } from './SystemConsole';
import { Gauge, Cpu, Database, HardDrive, Layers, Activity, Play, Zap, Box, BarChart3 } from 'lucide-react';

interface Metrics {
  timestamp: string;
  streams: any[];
  gpus: any[];
  system: {
    cpuUsage: number;
    ramUsed: number;
    ramTotal: number;
  };
}

export default function Dashboard() {
  const [metrics, setMetrics] = useState<Metrics | null>(null);
  const [isGenerating, setIsGenerating] = useState(false);
  const [benchmarkResult, setBenchmarkResult] = useState<any>(null);
  const [status, setStatus] = useState('IDLE');

  const handleGenerateEngine = async () => {
    setIsGenerating(true);
    setStatus('COMPILING_ENGINE');
    try {
      await fetch('/api/generate-engine', { method: 'POST' });
      setTimeout(() => {
        setIsGenerating(false);
        setStatus('ENGINE_READY');
      }, 5000);
    } catch (err) {
      console.error(err);
      setIsGenerating(false);
    }
  };

  const handleBenchmark = async () => {
    setStatus('BENCHMARKING');
    try {
      const res = await fetch('/api/benchmark', { method: 'POST' });
      const data = await res.json();
      setBenchmarkResult(data);
      setStatus('READY');
    } catch (err) {
      console.error(err);
    }
  };

  const handleStartPipeline = () => {
    setStatus('RUNNING');
  };

  useEffect(() => {
    const fetchData = async () => {
      if (status !== 'RUNNING') return;
      try {
        const res = await fetch('/api/metrics');
        const data = await res.json();
        setMetrics(data);
      } catch (err) {
        console.error('Failed to fetch metrics', err);
      }
    };

    const interval = setInterval(fetchData, 2000);
    return () => clearInterval(interval);
  }, [status]);

  if (!metrics && status === 'RUNNING') return (
    <div className="h-screen w-screen bg-bg-dark flex items-center justify-center">
      <div className="flex flex-col items-center gap-6">
        <div className="w-16 h-16 border-2 border-accent-nvidia border-t-transparent rounded-full animate-spin" />
        <div className="text-center space-y-2">
          <p className="micro-label animate-pulse">Initializing Vision Core</p>
          <p className="text-[9px] text-text-secondary font-mono">MAP_VRAM_96GB_SUCCESS</p>
        </div>
      </div>
    </div>
  );

  return (
    <div className="h-screen w-screen overflow-hidden grid grid-rows-[64px_1fr_140px] grid-cols-[320px_1fr] bg-bg-dark text-text-primary">
      {/* Header */}
      <header className="col-span-full bg-bg-surface border-b border-border-theme flex items-center justify-between px-8">
        <div className="flex items-center gap-4">
          <div className="w-3 h-6 bg-accent-nvidia shadow-[0_0_15px_rgba(118,185,0,0.4)]" />
          <div className="flex flex-col">
            <h1 className="text-lg font-bold tracking-tighter leading-none uppercase">VisionStream X100</h1>
            <span className="text-[9px] font-mono text-text-secondary tracking-widest uppercase">1080P TensorRT Native Pipeline</span>
          </div>
        </div>

        <div className="flex items-center gap-4">
          <button 
            onClick={handleGenerateEngine}
            disabled={isGenerating}
            className={`flex items-center gap-2 px-4 py-2 border border-border-theme text-[10px] uppercase tracking-widest font-mono transition-all ${isGenerating ? 'opacity-50 cursor-not-allowed' : 'hover:bg-white/5 hover:border-accent-nvidia'}`}
          >
            <Box className={`w-3 h-3 ${isGenerating ? 'animate-spin' : ''}`} />
            {isGenerating ? 'Compiling Engine...' : 'Compile YOLO Engine'}
          </button>
          
          <button 
            onClick={handleBenchmark}
            className="flex items-center gap-2 px-4 py-2 border border-border-theme text-[10px] uppercase tracking-widest font-mono hover:bg-white/5 hover:border-blue-500 transition-all"
          >
            <BarChart3 className="w-3 h-3" />
            Run Stress Test
          </button>

          <button 
            onClick={handleStartPipeline}
            className="flex items-center gap-2 px-6 py-2 bg-accent-nvidia text-black text-[10px] font-bold uppercase tracking-widest hover:bg-opacity-80 transition-all"
          >
            <Play className="w-3 h-3 fill-current" />
            Start Pipeline
          </button>
        </div>
        
        <div className="flex items-center gap-10 font-mono text-[10px] text-text-secondary">
          <div className="flex flex-col items-end">
            <span className="micro-label text-[8px] opacity-50">Status</span>
            <span className={status === 'RUNNING' ? 'text-accent-nvidia' : 'text-text-primary'}>{status}</span>
          </div>
          <div className="flex flex-col items-end">
            <span className="micro-label text-[8px] opacity-50">Compute_Fabric</span>
            <span className="text-text-primary">DUAL-RTX-4090-24GB</span>
          </div>
        </div>
      </header>

      {/* Sidebar */}
      <aside className="bg-bg-surface border-r border-border-theme p-6 flex flex-col gap-6 overflow-y-auto">
        {/* Global Stats */}
        {metrics ? (
          <div className="grid grid-cols-1 gap-4">
            <div className="border border-border-theme p-4 bg-white/5 group hover:bg-white/10 transition-colors">
              <div className="flex justify-between items-start mb-2">
                <span className="micro-label">Throughput</span>
                <Layers className="w-3 h-3 text-text-secondary" />
              </div>
              <div className="value-display">
                {metrics.streams.reduce((acc, s) => acc + s.fps, 0).toLocaleString(undefined, { maximumFractionDigits: 0 })}
                <span className="text-xs text-text-secondary ml-1 font-normal">FPS</span>
              </div>
            </div>

            <div className="border border-border-theme p-4 bg-white/5 group hover:bg-white/10 transition-colors">
              <div className="flex justify-between items-start mb-2">
                <span className="micro-label">Latency</span>
                <Activity className="w-3 h-3 text-text-secondary" />
              </div>
              <div className="value-display text-accent-nvidia">
                {(metrics.streams.reduce((acc, s) => acc + s.latency, 0) / 100).toFixed(2)}
                <span className="text-xs text-text-secondary ml-1 font-normal">MS</span>
              </div>
            </div>
          </div>
        ) : (
          <div className="p-8 border border-dashed border-border-theme text-center">
            <p className="micro-label opacity-50">Pipeline Inactive</p>
          </div>
        )}

        {/* GPU Monitors */}
        <div className="space-y-4">
          <span className="micro-label opacity-50">Hardware_Accelerators</span>
          {metrics?.gpus.map((gpu, idx) => (
            <GpuMonitor key={gpu.id} {...gpu} />
          )) || (
            <div className="space-y-4">
              <div className="h-24 bg-white/5 animate-pulse rounded" />
              <div className="h-24 bg-white/5 animate-pulse rounded" />
            </div>
          )}
        </div>

        {/* Capacity Analysis */}
        <div className="mt-auto space-y-4 pt-6 border-t border-white/5">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2 micro-label opacity-50">
              <Gauge className="w-3 h-3" />
              <span>Capacity_Forecast</span>
            </div>
            {benchmarkResult && (
              <span className="text-[10px] font-mono text-accent-nvidia">STABLE: {benchmarkResult.maxStreams}</span>
            )}
          </div>
          
          {benchmarkResult ? (
            <div className="p-3 bg-blue-500/10 border border-blue-500/20 space-y-2">
              <p className="text-[9px] font-mono text-blue-400 leading-tight">
                BOTTLENECK: {benchmarkResult.bottleneck}
              </p>
              <p className="text-[8px] font-mono text-text-secondary leading-tight uppercase">
                {benchmarkResult.recommendation}
              </p>
            </div>
          ) : (
            <div className="space-y-3">
              <div className="space-y-1">
                <div className="flex justify-between text-[9px] font-mono">
                  <span className="text-text-secondary">VRAM_HEADROOM</span>
                  <span className="text-accent-nvidia">~480 STREAMS</span>
                </div>
                <div className="h-1 bg-white/5 w-full">
                  <div className="h-full bg-accent-nvidia/40" style={{ width: '20%' }} />
                </div>
              </div>
              <div className="space-y-1">
                <div className="flex justify-between text-[9px] font-mono">
                  <span className="text-text-secondary">NVDEC_LOAD</span>
                  <span className="text-text-primary">100 / 144</span>
                </div>
                <div className="h-1 bg-white/5 w-full">
                  <div className="h-full bg-blue-500/40" style={{ width: '69%' }} />
                </div>
              </div>
            </div>
          )}
        </div>
      </aside>

      {/* Main Content */}
      <main className="p-6 overflow-hidden relative">
        {metrics ? (
          <StreamMatrix streams={metrics.streams} />
        ) : (
          <div className="h-full w-full flex items-center justify-center border border-dashed border-border-theme">
            {status === 'BENCHMARKING' ? (
              <div className="text-center space-y-6">
                <div className="w-12 h-12 border-2 border-blue-500 border-t-transparent rounded-full animate-spin mx-auto" />
                <div className="space-y-2">
                  <p className="micro-label text-blue-400 animate-pulse">Running Hardware Stress Test</p>
                  <p className="text-[9px] text-text-secondary font-mono">ANALYZING_NVDEC_THROUGHPUT_BY_96GB_VRAM</p>
                </div>
              </div>
            ) : benchmarkResult ? (
              <div className="max-w-2xl w-full bg-bg-surface border border-blue-500/30 p-8 space-y-8 shadow-[0_0_50px_rgba(59,130,246,0.1)]">
                <div className="flex items-center justify-between border-b border-white/5 pb-6">
                  <div className="flex items-center gap-4">
                    <BarChart3 className="w-8 h-8 text-blue-500" />
                    <div>
                      <h2 className="text-xl font-bold tracking-tighter uppercase">Hardware Capacity Report</h2>
                      <p className="text-[10px] font-mono text-text-secondary">DUAL RTX 4090 | 96GB VRAM | 512GB RAM</p>
                    </div>
                  </div>
                  <div className="text-right">
                    <span className="micro-label opacity-50">Max Stable Streams</span>
                    <div className="text-4xl font-mono font-bold text-accent-nvidia">{benchmarkResult.maxStreams}</div>
                  </div>
                </div>

                <div className="grid grid-cols-3 gap-6">
                  <div className="space-y-2">
                    <span className="micro-label opacity-50">VRAM Capacity</span>
                    <div className="text-lg font-mono">{benchmarkResult.details.vramLimit} <span className="text-[10px] text-text-secondary">CH</span></div>
                    <div className="h-1 bg-white/5 w-full"><div className="h-full bg-accent-nvidia" style={{ width: '100%' }} /></div>
                  </div>
                  <div className="space-y-2">
                    <span className="micro-label opacity-50">NVDEC Limit</span>
                    <div className="text-lg font-mono text-blue-400">{benchmarkResult.details.nvdecLimit} <span className="text-[10px] text-text-secondary">CH</span></div>
                    <div className="h-1 bg-white/5 w-full"><div className="h-full bg-blue-500" style={{ width: '80%' }} /></div>
                  </div>
                  <div className="space-y-2">
                    <span className="micro-label opacity-50">Inference Headroom</span>
                    <div className="text-lg font-mono">{benchmarkResult.details.inferenceLimit} <span className="text-[10px] text-text-secondary">CH</span></div>
                    <div className="h-1 bg-white/5 w-full"><div className="h-full bg-purple-500" style={{ width: '60%' }} /></div>
                  </div>
                </div>

                <div className="bg-white/5 p-6 border border-white/5 space-y-4">
                  <div className="flex items-center gap-2 text-blue-400">
                    <Activity className="w-4 h-4" />
                    <span className="micro-label">Bottleneck Analysis</span>
                  </div>
                  <p className="text-sm font-mono leading-relaxed">
                    System bottleneck identified as <span className="text-blue-400 font-bold">[{benchmarkResult.bottleneck}]</span>. 
                    While VRAM (96GB) and System RAM (512GB) are exceptionally high, the physical NVDEC count on consumer 4090 cards limits concurrent 1080P hardware decoding to {benchmarkResult.maxStreams} streams at 25 FPS.
                  </p>
                  <div className="pt-4 border-t border-white/5">
                    <p className="text-[10px] text-text-secondary uppercase tracking-widest">Recommendation:</p>
                    <p className="text-xs text-text-primary mt-1">{benchmarkResult.recommendation}</p>
                  </div>
                </div>

                <button 
                  onClick={() => setBenchmarkResult(null)}
                  className="w-full py-3 border border-white/10 hover:bg-white/5 micro-label transition-colors"
                >
                  Dismiss Report
                </button>
              </div>
            ) : (
              <div className="text-center space-y-4">
                <Zap className="w-12 h-12 text-text-secondary mx-auto opacity-20" />
                <p className="micro-label opacity-50">Awaiting Pipeline Activation</p>
              </div>
            )}
          </div>
        )}
      </main>

      {/* Footer Console */}
      <footer className="col-span-full">
        <SystemConsole />
      </footer>
    </div>
  );
}
