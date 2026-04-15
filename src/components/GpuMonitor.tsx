import React from 'react';
import { Zap, Thermometer, Activity } from 'lucide-react';
import { cn } from '../lib/utils';

interface GpuProps {
  id: number;
  model: string;
  utilization: number;
  vramUsed: number;
  vramTotal: number;
  temp: number;
  power: number;
}

export const GpuMonitor: React.FC<GpuProps> = ({ id, model, utilization, vramUsed, vramTotal, temp, power }) => {
  return (
    <div className="border border-border-theme p-4 bg-white/5 space-y-4">
      <div className="flex justify-between items-center">
        <div className="flex items-center gap-2">
          <Zap className="w-4 h-4 text-accent-nvidia" />
          <span className="micro-label">GPU {id}: {model}</span>
        </div>
        <span className="font-mono text-[10px] text-text-secondary">{temp}°C</span>
      </div>

      <div className="grid grid-cols-2 gap-4">
        <div className="space-y-1">
          <div className="flex justify-between text-[10px] text-text-secondary uppercase">
            <span>Utilization</span>
            <span>{utilization.toFixed(0)}%</span>
          </div>
          <div className="h-1 bg-white/10 w-full">
            <div 
              className="h-full bg-accent-nvidia transition-all duration-500" 
              style={{ width: `${utilization}%` }}
            />
          </div>
        </div>

        <div className="space-y-1">
          <div className="flex justify-between text-[10px] text-text-secondary uppercase">
            <span>VRAM</span>
            <span>{vramUsed.toFixed(1)}G</span>
          </div>
          <div className="h-1 bg-white/10 w-full">
            <div 
              className="h-full bg-blue-500 transition-all duration-500" 
              style={{ width: `${(vramUsed / vramTotal) * 100}%` }}
            />
          </div>
        </div>
      </div>

      <div className="flex justify-between items-center pt-2 border-t border-white/5">
        <div className="flex items-center gap-1.5 text-[10px] text-text-secondary">
          <Activity className="w-3 h-3" />
          <span>{power.toFixed(0)}W</span>
        </div>
        <div className="flex items-center gap-1.5 text-[10px] text-text-secondary">
          <Thermometer className="w-3 h-3" />
          <span>Core: {temp}°C</span>
        </div>
      </div>
    </div>
  );
};
