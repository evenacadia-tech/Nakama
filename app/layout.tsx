import type { Metadata } from 'next';
import { Geist, Geist_Mono } from 'next/font/google';
import './globals.css';

const geistSans = Geist({
  variable: '--font-geist-sans',
  subsets: ['latin'],
});

const geistMono = Geist_Mono({
  variable: '--font-geist-mono',
  subsets: ['latin'],
});

export const metadata: Metadata = {
  metadataBase: new URL('https://nakama-briefing.philipld.chatgpt.site'),
  title: 'Nakama Briefing',
  description: 'Der gemeinsame Arbeitsstand für Phil, Claude und Codex.',
  openGraph: {
    title: 'Nakama Briefing',
    description: 'Entscheidungen, Stand und nächste Schritte.',
    url: 'https://nakama-briefing.philipld.chatgpt.site',
    siteName: 'Nakama Briefing',
    locale: 'de_DE',
    type: 'website',
    images: [
      {
        url: 'https://nakama-briefing.philipld.chatgpt.site/og.png',
        width: 1200,
        height: 630,
        alt: 'Nakama Briefing: Entscheidungen, Stand und nächste Schritte.',
      },
    ],
  },
  twitter: {
    card: 'summary_large_image',
    title: 'Nakama Briefing',
    description: 'Entscheidungen, Stand und nächste Schritte.',
    images: ['https://nakama-briefing.philipld.chatgpt.site/og.png'],
  },
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="de">
      <body className={`${geistSans.variable} ${geistMono.variable}`}>
        {children}
      </body>
    </html>
  );
}
