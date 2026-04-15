import React from 'react';
import { cn } from '../lib/utils';

interface Stream {
  id: number;
  status: 'active' | 'error';
  fps: number;
  latency: number;
}

export const StreamMatrix: React.FC<{ streams: Stream[] }> = ({ streams }) => {
  return (
    <div className="grid grid-cols-10 grid-rows-10 gap-1.5 h-full">
      {streams.map((stream) => (
        <div 
          key={stream.id}
          className={cn(
            "border bg-[#111] relative flex flex-col justify-end p-1 transition-all duration-300 group",
            stream.status === 'active' 
              ? "border-accent-nvidia/20 hover:border-accent-nvidia/60" 
              : "border-accent-alert/40 bg-accent-alert/5"
          )}
        >
          <div className="absolute top-1 left-1 text-[8px] font-mono text-text-secondary opacity-50">
            S-{(stream.id + 1).toString().padStart(3, '0')}
          </div>
          
          {stream.status === 'active' && (
            <div className="absolute top-1.5 right-1.5 w-1 h-1 rounded-full bg-accent-nvidia shadow-[0_0_4px_rgba(118,185,0,0.8)]" />
          )}

          <div className="flex justify-between items-baseline text-[7px] font-mono leading-none">
            <span className="text-text-secondary">1080P</span>
            <span className={cn(
              stream.status === 'active' ? "text-text-primary" : "text-accent-alert"
            )}>
              {stream.fps.toFixed(0)}F
            </span>
          </div>
          
          {/* Hover Detail */}
          <div className="absolute inset-0 bg-bg-dark/95 opacity-0 group-hover:opacity-100 transition-opacity flex flex-col justify-center items-center gap-1 z-10 pointer-events-none">
            <span className="text-[9px] font-bold text-accent-nvidia">STREAM #{stream.id + 1}</span>
            <span className="text-[8px] font-mono text-text-secondary">{stream.latency.toFixed(2)}ms</span>
          </div>
        </div>
      ))}
    </div>
  );
};
