using SkiaSharp;

class Program
{
	static void Main(){
		int width = 800, height = 600;	
		using var surface = SKSurface.Create(new SKImageInfo(width, height));
		var canvas = surface.Canvas;
		canvas.Clear(SKColor.Parse("#003366"));
		using var bitmap = SKBitmap.Decode("OSAI.png");

		for(var y = 0; y < bitmap.Height; y++){
			for(var x = 0; x < bitmap.Width; x++){
				var pixelColor = bitmap.GetPixel(x, y);	
				System.Console.WriteLine(pixelColor);

				//SKColor has ４ parameter may be RGB?
				var modColor = new SKColor(
					red: (byte)(x % 255),			
					green: (byte)(y % 255),
					blue: (byte)(x + y % 255),
					alpha: pixelColor.Alpha);

				var rectPaint = new SKPaint {Color = modColor};
				canvas.DrawRect(x, y, 1, 1, rectPaint);
			}
		}
		using var image = surface.Snapshot();
		using var data = image.Encode(SKEncodedImageFormat.Png, 100);

		const string outputPath = "out.png";
		using(var stream = File.OpenWrite(outputPath))
		{
			data.SaveTo(stream);
		}

		System.Console.WriteLine($"saved image:{outputPath}");
	}
	
}
