import React, { useEffect, useRef, useState } from 'react';
import { Terminal } from 'lucide-react';

export const SystemConsole: React.FC = () => {
  const [logs, setLogs] = useState<{ time: string, msg: string, type: string }[]>([]);
  const scrollRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const messages = [
      { type: 'CUDA', msg: 'Buffer mapped for GPU:0 memory handle 0x7f88...' },
      { type: 'TENSORRT', msg: 'Engine context initialized: ResNet50_Backbone_v4' },
      { type: 'SYSTEM', msg: 'Streaming thread pool spawned (100 workers)' },
      { type: 'INFERENCE', msg: 'Global frame synchronization active: 100/100 channels' },
      { type: 'NVDEC', msg: 'Hardware decoder session established for stream #042' },
    ];

    const interval = setInterval(() => {
      const randomMsg = messages[Math.floor(Math.random() * messages.length)];
      setLogs(prev => [...prev.slice(-50), {
        time: new Date().toLocaleTimeString(),
        ...randomMsg
      }]);
    }, 3000);

    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    if (scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [logs]);

  return (
    <div className="bg-[#050505] border-t border-border-theme h-full flex flex-col">
      <div className="px-6 py-2 border-b border-border-theme flex items-center gap-2 bg-bg-surface/50">
        <Terminal className="w-3 h-3 text-text-secondary" />
        <span className="micro-label text-[9px]">Kernel_Output_Stream</span>
      </div>
      <div 
        ref={scrollRef}
        className="flex-1 overflow-y-auto p-4 font-mono text-[10px] space-y-1 selection:bg-accent-nvidia/30"
      >
        {logs.map((log, i) => (
          <div key={i} className="flex gap-3 opacity-80 hover:opacity-100 transition-opacity">
            <span className="text-text-secondary">[{log.time}]</span>
            <span className="text-accent-nvidia font-bold">[{log.type}]</span>
            <span className="text-[#00FF41]">{log.msg}</span>
          </div>
        ))}
        {logs.length === 0 && <div className="text-text-secondary animate-pulse">Waiting for kernel data...</div>}
      </div>
    </div>
  );
};
