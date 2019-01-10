package senpai.table;

public enum TypeAtome {
	Redium(4,1),
	Greenium(8,1),
	Blueium(12,1),
	Goldenium(24,1);
	
	public final int nbPoints;
	public final int taille;
	
	private TypeAtome(int nbPoints, int taille)
	{
		this.nbPoints = nbPoints;
		this.taille = taille;
	}
}
