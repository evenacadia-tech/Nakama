import BriefingApp from './briefing-app';
import type { HubData } from './briefing-app';
import hub from '@/data/hub.json';

export default function Home() {
  return <BriefingApp initialHub={hub as HubData} />;
}
